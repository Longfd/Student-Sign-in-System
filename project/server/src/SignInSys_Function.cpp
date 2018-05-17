#include <sstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <map>

#include "SignInSys_Function.h"
#include "SignInSys_db.h"
#include "oradb.h"
#include "packet.h"
#include "loginfo.h"
#include "comm_thread.h"


extern int g_conn_count;
extern OraConn* g_conn_list;

using json = nlohmann::json;


//1.用户注册 Begin
int insertTeachOrStu(int conn_no, const person& person, std::string& err)
{
	int iRet;
	MYSQL* mysql_conn = g_conn_list[conn_no].dbc_p;
	std::ostringstream sql_stmt;
	char errbuf[SQL_ERR_BUF_SIZE];
	int error_no;
	const char* funcName = "insertTeachOrStu";

	if (NULL == g_conn_list || conn_no<0 || conn_no >= g_conn_count)	return -1;

	if (0 == person.role.compare("1")){
		sql_stmt << "INSERT INTO TEACHER_TBL(t_id,t_name,t_passwd) VALUES('";
	}
	else if (0 == person.role.compare("0")){
		sql_stmt << "INSERT INTO STUDENT_TBL(s_id,s_name,s_passwd) VALUES('";
	}
	else{
		return -2;
	}

	sql_stmt << person.userId << "','"
		<< person.userName << "','"
		<< person.pwd << "');";
	write_debug_log("In Func[%s] : SQL:%s", funcName, sql_stmt.str().c_str());

	iRet = mysql_query(mysql_conn, sql_stmt.str().c_str());
	if (0 != iRet){
		get_conn_error(&g_conn_list[conn_no], &error_no, errbuf, SQL_ERR_BUF_SIZE);
		if (2006 == error_no){
			write_debug_log("In Func[%s] : execute_stmt() fail ! 数据库连接中断，重新启动服务!", funcName);
			exit(error_no);
		}
		write_debug_log("In Func[%s] : execute_stmt() fail ! 错误码:%d, 详细信息:%s", funcName, error_no, errbuf);
		err = std::string(errbuf);
		return -3;
	}

	return 0;
}

int insertPerson(const person& person, std::string& err)
{
	const char* funcName = "insertPerson";

	if (0 == person.userId.size() || 0 == person.pwd.size() ||
		0 == person.role.size() || 0 == person.userName.size()) {
		err = std::string("无效数据");
		write_debug_log("In Func[%s] 参数检查不通过!", funcName);
		return ERRNO_ILLEGAL_PARAM;
	}
		

	int conn_no = get_free_conn();
	if (conn_no < 0){
		err = std::string("无可用连接");
		write_debug_log("In Func[%s] : get_free_conn() fail", funcName);
		return -1;
	}

	int iRet = insertTeachOrStu(conn_no, person, err);
	if (iRet != 0){
		rollback_trans(conn_no);
		free_conn(conn_no);
		write_debug_log("In Func[%s] : insertTeachOrStu() fail, iRet:%d", funcName, iRet);
		return -2;
	}

	commit_trans(conn_no);
	free_conn(conn_no);
	return 0;
}

int userRegister(CommThreadInfo* thread_info, unsigned char* data)
{
	int iRet;
	person person;
	json jret;
	std::string str_data((char*)data);
	std::string str_ret;
	std::ostringstream oss_debug;
	const char* funcName = "userRegister";

	write_debug_log("In Func[%s] : 接收到注册请求报文, 包体内容:%s", funcName, data);

	try{
		// parse json
		json j1 = json::parse(str_data);
		person.from_json(j1);
		oss_debug << "UserId:" << person.userId << '\n' << "UserName:" << person.userName << '\n'
			<< "role:" << person.role << '\n' << "pwd:" << person.pwd << '\n';
		write_debug_log("In Func[%s] : Json 解析成功:%s", funcName, oss_debug.str().c_str());

		// dispose data
		std::string str_err;
		iRet = insertPerson(person, str_err);
		if (0 != iRet){
			jret[RESPON_CODE] = "1";
			jret[RESPON_MSG] = str_err.c_str();
			str_ret = jret.dump();
			write_debug_log("In Func[%s] : insertPerson() iRet:%d, str_err:%s", funcName, iRet, str_err.c_str());
		}
		else{
			jret[RESPON_CODE] = "0";
			jret[RESPON_MSG] = "success";
			str_ret = jret.dump();
			write_debug_log("In Func[%s] : insertPerson() success!", funcName);
		}	
	}// end try
	catch (...){
		jret[RESPON_CODE] = "1";
		jret[RESPON_MSG] = "JSON处理异常!";
		str_ret = jret.dump();
		write_debug_log("In Func[%s] : JSON处理异常!", funcName);
		iRet = send_packet(thread_info->comm_sock, CLIENT_REQ_REGISTER, str_ret.length(), (const unsigned char*)str_ret.c_str());
		if (iRet <= 0){
			write_debug_log("In Func[%s] : 发送应答包错误! iRet:%d, sys_err:%d", funcName, iRet, errno);
		}
		return iRet;
	}//end catch	

	iRet = send_packet(thread_info->comm_sock, CLIENT_REQ_REGISTER, str_ret.length(), (const unsigned char*)str_ret.c_str());
	if (iRet <= 0){
		write_debug_log("In Func[%s] : 发送应答包错误! iRet:%d, sys_err:%d", funcName, iRet, errno);
	}
	write_debug_log("In Func[%s] : 发送应答包成功! 发送总长:%d, 包体长:%d, 包体内容:%s", 
		funcName, iRet, str_ret.length(), str_ret.c_str());
	return iRet;
}
//1.用户注册 End

//2.用户登录 Begin
int queryPerson(person& person, std::string& err)
{
	int iRet;
	std::ostringstream sql_stmt;
	char errbuf[SQL_ERR_BUF_SIZE];
	int error_no;
	const char* funcName = "queryPerson";

	if (0 == person.userId.size() || 0 == person.pwd.size() || 0 == person.role.size()) {
		err = std::string("无效数据");
		write_debug_log("In Func[%s] 参数检查不通过!", funcName);
		return ERRNO_ILLEGAL_PARAM;
	}

	int conn_no = get_free_conn();
	if (conn_no < 0){
		err = std::string("无可用连接");
		write_debug_log("In Func[%s] : get_free_conn() fail", funcName);
		return -1;
	}

	if (NULL == g_conn_list || conn_no >= g_conn_count)	{
		free_conn(conn_no);
		return -2;
	}

	MYSQL* mysql_conn = g_conn_list[conn_no].dbc_p;

	if (0 == person.role.compare("1")){
		sql_stmt << "SELECT * FROM TEACHER_TBL WHERE t_id = '";
	}
	else if (0 == person.role.compare("0")){
		sql_stmt << "SELECT * FROM STUDENT_TBL WHERE s_id = '";
	}
	else{
		free_conn(conn_no);
		return -3;
	}
	sql_stmt << person.userId << "';";
	write_debug_log("In Func[%s] : SQL:%s", funcName, sql_stmt.str().c_str());

	iRet = mysql_refresh(mysql_conn, REFRESH_TABLES);
	if (0 != iRet){
		get_conn_error(&g_conn_list[conn_no], &error_no, errbuf, SQL_ERR_BUF_SIZE);
		if (2006 == error_no){
			write_debug_log("In Func[%s] : mysql_refresh() fail ! 数据库连接中断，重新启动服务!", funcName);
			exit(error_no);
		}
		write_debug_log("In Func[%s] : mysql_refresh() fail ! 错误码:%d, 详细信息:%s", funcName, error_no, errbuf);
		err = std::string(errbuf);
		free_conn(conn_no);
		return -4;
	}

	iRet = mysql_query(mysql_conn, sql_stmt.str().c_str());
	if (0 != iRet){
		get_conn_error(&g_conn_list[conn_no], &error_no, errbuf, SQL_ERR_BUF_SIZE);
		if (2006 == error_no){
			write_debug_log("In Func[%s] : mysql_query() fail ! 数据库连接中断，重新启动服务!", funcName);
			exit(error_no);
		}
		write_debug_log("In Func[%s] : mysql_query() fail ! 错误码:%d, 详细信息:%s", funcName, error_no, errbuf);
		err = std::string(errbuf);
		free_conn(conn_no);
		return -5;
	}

	MYSQL_RES* result = mysql_store_result(mysql_conn);
	int iCount = mysql_num_rows(result);
	if (iCount <= 0){
		write_debug_log("In Func[%s] : mysql_num_rows() fail ! iCount:%d, 用户不存在!", funcName, iCount);
		err = std::string("用户不存在!");
		free_conn(conn_no);
		return -6;
	}

	MYSQL_ROW row = NULL;
	row = mysql_fetch_row(result);
	if (row)
	{
		person.userName.assign(row[1]);
		if (0 == person.pwd.compare(row[2]))
		{
			write_debug_log("In Func[%s] : 用户验证通过! Role[%s]", funcName, person.role.c_str());
			if (0 == person.role.compare("0") && NULL != row[4])
			{
				write_debug_log("In Func[%s] : 添加学生信息! row3[%d], row4[%s]", funcName, person.role.c_str(), row[3], row[4]);
				//学生 添加班级信息
				std::ostringstream clsid; clsid << row[3];
				person.cls_id.assign(clsid.str());
				person.cls_name = std::string(row[4]);
			}
			free_conn(conn_no);
			mysql_free_result(result);
			return 0;
		}
	}
	err = std::string("用户验证不通过!");
	write_debug_log("In Func[%s] : 用户验证不通过! pwd[%s], pwdDB[%s]", funcName, person.pwd.c_str(), row[2]);
	free_conn(conn_no);
	if (result != NULL){
		mysql_free_result(result);
	}
	return -7;
}
int userSignUp(CommThreadInfo* thread_info, unsigned char* data)
{
	int iRet;
	person person;
	json jret;
	std::string str_data((char*)data);
	std::string str_ret;
	std::ostringstream oss_debug;
	const char* funcName = "userSignUp";

	write_debug_log("In Func[%s] : 接收到用户登录请求报文, 包体内容:%s", funcName, data);

	try{
		// parse json
		json j1 = json::parse(str_data);
		/*oss_debug << "json:" << j1 << '\n';
		write_debug_log("In Func[%s] : Json 解析成功:%s", funcName, oss_debug.str().c_str());*/

		person.from_json(j1);
		oss_debug << "UserId:" << person.userId << '\n' << "UserName:" << person.userName << '\n'
			<< "role:" << person.role << '\n' << "pwd:" << person.pwd << '\n';
		write_debug_log("In Func[%s] : Json 解析成功:%s", funcName, oss_debug.str().c_str());

		// dispose data
		std::string str_err;
		iRet = queryPerson(person, str_err);
		if (0 != iRet){
			jret[RESPON_CODE] = "1";
			jret[RESPON_MSG] = str_err.c_str();
			str_ret = jret.dump();
			write_debug_log("In Func[%s] : queryPerson() iRet:%d, str_err:%s", funcName, iRet, str_err.c_str());
		}
		else{
			jret[RESPON_CODE] = "0";
			jret[RESPON_MSG] = "success";
			jret[PERSON_ID] = person.userId.c_str();
			jret[PERSON_NAME] = person.userName.c_str();
			jret[PERSON_ROLE] = person.role.c_str();
			jret[CLS_ID] = person.cls_id.c_str();
			jret[CLS_NAME] = person.cls_name.c_str();
			str_ret = jret.dump();
			write_debug_log("In Func[%s] : queryPerson() success!", funcName);
		}
	}// end try
	catch (...){
		jret[RESPON_CODE] = "1";
		jret[RESPON_MSG] = "JSON处理异常!";
		str_ret = jret.dump();
		write_debug_log("In Func[%s] : JSON处理异常!", funcName);
		iRet = send_packet(thread_info->comm_sock, CLIENT_REQ_LOG_IN, str_ret.length(), (const unsigned char*)str_ret.c_str());
		if (iRet <= 0){
			write_debug_log("In Func[%s] : 发送应答包错误! iRet:%d, sys_err:%d", funcName, iRet, errno);
		}
		return iRet;
	}//end catch	

	iRet = send_packet(thread_info->comm_sock, CLIENT_REQ_LOG_IN, str_ret.length(), (const unsigned char*)str_ret.c_str());
	if (iRet <= 0){
		write_debug_log("In Func[%s] : 发送应答包错误! iRet:%d, sys_err:%d", funcName, iRet, errno);
	}
	write_debug_log("In Func[%s] : 发送应答包成功! 发送总长:%d, 包体长:%d, 包体内容:%s",
		funcName, iRet, str_ret.length(), str_ret.c_str());
	return iRet;
}
//2.用户登录 End

//3.增加班级 Begin
int insertClsSqlOpt(int conn_no, const classInfo& cls, std::string& err)
{
	int iRet;
	MYSQL* mysql_conn = g_conn_list[conn_no].dbc_p;
	std::ostringstream sql_stmt;
	char errbuf[SQL_ERR_BUF_SIZE];
	int error_no;
	const char* funcName = "insertClsSqlOpt";

	if (NULL == g_conn_list || conn_no < 0 || conn_no >= g_conn_count) {
		err = std::string("无效数据");
		write_debug_log("In Func[%s] 参数检查不通过!", funcName);
		return ERRNO_ILLEGAL_PARAM;
	}

	sql_stmt << "INSERT INTO CLASS_TBL(cls_name,t_id) VALUES('";

	sql_stmt << cls.clsName << "','"
		<< cls.t_id << "');";
	write_debug_log("In Func[%s] : SQL:%s", funcName, sql_stmt.str().c_str());

	iRet = mysql_query(mysql_conn, sql_stmt.str().c_str());
	if (0 != iRet){
		get_conn_error(&g_conn_list[conn_no], &error_no, errbuf, SQL_ERR_BUF_SIZE);
		if (2006 == error_no){
			write_debug_log("In Func[%s] : execute_stmt() fail ! 数据库连接中断，重新启动服务!", funcName);
			exit(error_no);
		}
		write_debug_log("In Func[%s] : execute_stmt() fail ! 错误码:%d, 详细信息:%s", funcName, error_no, errbuf);
		err = std::string(errbuf);
		return -3;
	}

	return 0;
}
int insertCls(const classInfo& cls, std::string& err)
{
	const char* funcName = "insertCls";

	if (0 == cls.t_id.size() || 0 == cls.clsName.size())  {
		err = std::string("无效数据");
		write_debug_log("In Func[%s] 参数检查不通过!", funcName);
		return ERRNO_ILLEGAL_PARAM;
	}

	int conn_no = get_free_conn();
	if (conn_no < 0){
		err = std::string("无可用连接");
		write_debug_log("In Func[%s] : get_free_conn() fail", funcName);
		return -1;
	}

	int iRet = insertClsSqlOpt(conn_no, cls, err);
	if (iRet != 0){
		rollback_trans(conn_no);
		free_conn(conn_no);
		write_debug_log("In Func[%s] : insertTeachOrStu() fail, iRet:%d", funcName, iRet);
		return -2;
	}

	commit_trans(conn_no);
	free_conn(conn_no);
	return 0;
}
int addClassRequest(CommThreadInfo* thread_info, unsigned char* data)
{
	int iRet;
	classInfo cls;
	json jret;
	std::string str_data((char*)data);
	std::string str_ret;
	std::ostringstream oss_debug;
	const char* funcName = "addClassRequest";

	write_debug_log("In Func[%s] : 接收到增加班级请求报文, 包体内容:%s", funcName, data);

	try{
		// parse json
		json j1 = json::parse(str_data);
		cls.from_json(j1);
		oss_debug << "cls_name:" << cls.clsName << '\n' << "t_id:" << cls.t_id << '\n';
		write_debug_log("In Func[%s] : Json 解析成功:%s", funcName, oss_debug.str().c_str());

		// dispose data
		std::string str_err;
		iRet = insertCls(cls, str_err);
		if (0 != iRet){
			jret["result"] = "1";
			jret["msg"] = str_err.c_str();
			str_ret = jret.dump();
			write_debug_log("In Func[%s] : insertCls() fail ! iRet:%d, str_err:%s", funcName, iRet, str_err.c_str());
		}
		else{
			jret["result"] = "0";
			jret["msg"] = "success";
			str_ret = jret.dump();
			write_debug_log("In Func[%s] : insertCls() success!", funcName);
		}
	}// end try
	catch (...){
		jret["result"] = "1";
		jret["msg"] = "JSON处理异常!";
		str_ret = jret.dump();
		write_debug_log("In Func[%s] : JSON处理异常!", funcName);
		iRet = send_packet(thread_info->comm_sock, CLIENT_REQ_ADD_CLASS, str_ret.length(), (const unsigned char*)str_ret.c_str());
		if (iRet <= 0){
			write_debug_log("In Func[%s] : 发送应答包错误! iRet:%d, sys_err:%d", funcName, iRet, errno);
		}
		return iRet;
	}//end catch	

	iRet = send_packet(thread_info->comm_sock, CLIENT_REQ_ADD_CLASS, str_ret.length(), (const unsigned char*)str_ret.c_str());
	if (iRet <= 0){
		write_debug_log("In Func[%s] : 发送应答包错误! iRet:%d, sys_err:%d", funcName, iRet, errno);
	}
	write_debug_log("In Func[%s] : 发送应答包成功! 发送总长:%d, 包体长:%d, 包体内容:%s", funcName, iRet, str_ret.length(), str_ret.c_str());
	return iRet;
}
//3.增加班级 End

//4.加入班级 Begin
int updateStuSqlOpt(int conn_no, stuAndClsMap& mapInfo, std::string& err)
{
	int iRet;
	MYSQL* mysql_conn = g_conn_list[conn_no].dbc_p;
	std::ostringstream sql_stmt;
	char errbuf[SQL_ERR_BUF_SIZE];
	int error_no;
	const char* funcName = "updateStuSqlOpt";

	if (NULL == g_conn_list || conn_no < 0 || conn_no >= g_conn_count)	return -1;

	//查班级信息
	sql_stmt << "SELECT * FROM CLASS_TBL WHERE cls_no = " << mapInfo.cls_no;
	write_debug_log("In Func[%s] : SQL:%s", funcName, sql_stmt.str().c_str());

	iRet = mysql_refresh(mysql_conn, REFRESH_TABLES);
	iRet = mysql_query(mysql_conn, sql_stmt.str().c_str());
	if (0 != iRet){
		get_conn_error(&g_conn_list[conn_no], &error_no, errbuf, SQL_ERR_BUF_SIZE);
		if (2006 == error_no){
			write_debug_log("In Func[%s] : mysql_query() fail ! 数据库连接中断，重新启动服务!", funcName);
			exit(error_no);
		}
		write_debug_log("In Func[%s] : mysql_query() fail ! 错误码:%d, 详细信息:%s", funcName, error_no, errbuf);
		err = std::string(errbuf);
		return -2;
	}
	MYSQL_RES* result = mysql_store_result(mysql_conn);
	int iCount = mysql_num_rows(result);
	MYSQL_ROW row = mysql_fetch_row(result);
	if (iCount <= 0 || NULL == row){
		write_debug_log("In Func[%s] : mysql_num_rows() fail ! iCount:%d,班级不存在!", funcName, iCount);
		err = std::string("班级不存在!");
		return -3;
	}
	mapInfo.cls_name.assign(row[1]);
	
	//更新学生表
	sql_stmt.str("");
	sql_stmt << "UPDATE STUDENT_TBL SET cls_no = " << row[0] << ", cls_name = '"
		<< row[1] << "' WHERE s_id = '" << mapInfo.s_id << "';";
	write_debug_log("In Func[%s] : SQL:%s", funcName, sql_stmt.str().c_str());

	iRet = mysql_query(mysql_conn, sql_stmt.str().c_str());
	if (0 != iRet){
		get_conn_error(&g_conn_list[conn_no], &error_no, errbuf, SQL_ERR_BUF_SIZE);
		if (2006 == error_no){
			write_debug_log("In Func[%s] : mysql_query() fail ! 数据库连接中断，重新启动服务!", funcName);
			exit(error_no);
		}
		write_debug_log("In Func[%s] : mysql_query() fail ! 错误码:%d, 详细信息:%s", funcName, error_no, errbuf);
		err = std::string(errbuf);
		return -4;
	}

	return 0;
}

int updateStudent(stuAndClsMap& mapInfo, std::string& err)
{
	const char* funcName = "updateStudent";

	if (0 == mapInfo.cls_no.size() || 0 == mapInfo.s_id.size())  {
		err = std::string("无效数据");
		write_debug_log("In Func[%s] 参数检查不通过!", funcName);
		return ERRNO_ILLEGAL_PARAM;
	}

	int conn_no = get_free_conn();
	if (conn_no < 0){
		err = std::string("无可用连接");
		write_debug_log("In Func[%s] : get_free_conn() fail", funcName);
		return -1;
	}

	int iRet = updateStuSqlOpt(conn_no, mapInfo, err);
	if (iRet != 0){
		rollback_trans(conn_no);
		free_conn(conn_no);
		write_debug_log("In Func[%s] : updateStuSqlOpt() fail, iRet:%d", funcName, iRet);
		return -2;
	}

	commit_trans(conn_no);
	free_conn(conn_no);
	return 0;
}

int joinClassRequest(CommThreadInfo* thread_info, unsigned char* data)
{
	int iRet;
	stuAndClsMap mapInfo;
	json jret;
	std::string str_data((char*)data);
	std::string str_ret;
	std::ostringstream oss_debug;
	const char* funcName = "joinClassRequest";

	write_debug_log("In Func[%s] : 接收到加入班级请求报文, 包体内容:%s", funcName, data);

	try{
		// parse json
		json j1 = json::parse(str_data);
		mapInfo.from_json(j1);
		oss_debug << "userId:" << mapInfo.s_id << '\n' << "cls_no:" << mapInfo.cls_no << '\n';
		write_debug_log("In Func[%s] : Json 解析成功:%s", funcName, oss_debug.str().c_str());

		// dispose data
		std::string str_err;
		iRet = updateStudent(mapInfo, str_err);
		if (0 != iRet){
			jret["result"] = "1";
			jret["msg"] = str_err.c_str();
			str_ret = jret.dump();
			write_debug_log("In Func[%s] : updateStudent() fail ! iRet:%d, str_err:%s", funcName, iRet, str_err.c_str());
		}
		else{
			jret["result"] = "0";
			jret["msg"] = "success";
			jret[CLS_NAME] = mapInfo.cls_name;
			str_ret = jret.dump();
			write_debug_log("In Func[%s] : updateStudent() success!", funcName);
		}
	}// end try
	catch (...){
		jret["result"] = "1";
		jret["msg"] = "JSON处理异常!";
		str_ret = jret.dump();
		write_debug_log("In Func[%s] : JSON处理异常!", funcName);
		iRet = send_packet(thread_info->comm_sock, CLIENT_REQ_JOIN_CLASS, str_ret.length(), (const unsigned char*)str_ret.c_str());
		if (iRet <= 0){
			write_debug_log("In Func[%s] : 发送应答包错误! iRet:%d, sys_err:%d", funcName, iRet, errno);
		}
		return iRet;
	}//end catch	

	iRet = send_packet(thread_info->comm_sock, CLIENT_REQ_JOIN_CLASS, str_ret.length(), (const unsigned char*)str_ret.c_str());
	if (iRet <= 0){
		write_debug_log("In Func[%s] : 发送应答包错误! iRet:%d, sys_err:%d", funcName, iRet, errno);
	}
	write_debug_log("In Func[%s] : 发送应答包成功! 发送总长:%d, 包体长:%d, 包体内容:%s", funcName, iRet, str_ret.length(), str_ret.c_str());
	return iRet;
}
//4.加入班级 End

//5.查询班级 Begin
int queryClassInfoSqlOpt(const std::string& t_id, json& classInfoArray, std::string& err)
{
	int iRet;
	std::ostringstream sql_stmt;
	char errbuf[SQL_ERR_BUF_SIZE];
	int error_no;
	const char* funcName = "queryClassInfoSqlOpt";

	if (0 == t_id.size()) {
		err = std::string("无效数据");
		write_debug_log("In Func[%s] 参数检查不通过!", funcName);
		return ERRNO_ILLEGAL_PARAM;
	}

	int conn_no = get_free_conn();
	if (conn_no < 0 || NULL == g_conn_list || conn_no >= g_conn_count){
		err = std::string("无可用连接");
		write_debug_log("In Func[%s] : get_free_conn() fail", funcName);
		return -1;
	}

	MYSQL* mysql_conn = g_conn_list[conn_no].dbc_p;
	iRet = mysql_refresh(mysql_conn, REFRESH_TABLES);
	if (0 != iRet){
		get_conn_error(&g_conn_list[conn_no], &error_no, errbuf, SQL_ERR_BUF_SIZE);
		if (2006 == error_no){
			write_debug_log("In Func[%s] : mysql_refresh() fail ! 数据库连接中断，重新启动服务!", funcName);
			exit(error_no);
		}
		write_debug_log("In Func[%s] : mysql_refresh() fail ! 错误码:%d, 详细信息:%s", funcName, error_no, errbuf);
		err = std::string(errbuf);
		free_conn(conn_no);
		return -2;
	}

	sql_stmt << "SELECT * FROM CLASS_TBL WHERE t_id = '" << t_id << "';";
	write_debug_log("In Func[%s] : SQL:%s", funcName, sql_stmt.str().c_str());

	iRet = mysql_query(mysql_conn, sql_stmt.str().c_str());
	if (0 != iRet){
		get_conn_error(&g_conn_list[conn_no], &error_no, errbuf, SQL_ERR_BUF_SIZE);
		if (2006 == error_no){
			write_debug_log("In Func[%s] : mysql_query() fail ! 数据库连接中断，重新启动服务!", funcName);
			exit(error_no);
		}
		write_debug_log("In Func[%s] : mysql_query() fail ! 错误码:%d, 详细信息:%s", funcName, error_no, errbuf);
		err = std::string(errbuf);
		free_conn(conn_no);
		return -3;
	}

	MYSQL_RES* result = mysql_store_result(mysql_conn);
	int iCount = mysql_num_rows(result);
	MYSQL_ROW row = mysql_fetch_row(result);
	if (NULL == result || iCount <= 0 || NULL == row){
		write_debug_log("In Func[%s] : mysql_num_rows() fail ! iCount:%d, 用户未创建班级!", funcName, iCount);
		err = std::string("用户未创建班级!");
		free_conn(conn_no);
		return -4;
	}

	sql_stmt.str("");
	sql_stmt << "SELECT * FROM STUDENT_TBL WHERE cls_no in(";

	//存储班级信息 并准备下个查询语句
	std::vector<classWithStu> clsInfos_;
	std::map<std::string, std::string> cls_;
	for (int i = 0; i < iCount; ++i){
		classWithStu clsInfo;
		std::ostringstream itoa;
		itoa << row[0];
		clsInfo.cls_no = itoa.str();
		clsInfo.cls_name = row[1];
		clsInfos_.push_back(clsInfo);
		sql_stmt << itoa.str() << ",";
		row = mysql_fetch_row(result);
	}
	write_debug_log("In Func[%s] : 班级数据加载成功! 班级数:%d", funcName, iCount);
	mysql_free_result(result);

	sql_stmt << ");";
	//oss 返回的str是一个临时对象, 在上面无法删除字符
	std::string strSql = sql_stmt.str();
	size_t delPos = strSql.rfind(',');
	//erase(pos, num), 如果num不指定, 默认将pos后的字符都清除掉
	strSql.erase(delPos, 1);
	write_debug_log("In Func[%s] : SQL:%s", funcName, strSql.c_str());

	//查询学生
	iRet = mysql_query(mysql_conn, strSql.c_str());
	if (0 != iRet){
		get_conn_error(&g_conn_list[conn_no], &error_no, errbuf, SQL_ERR_BUF_SIZE);
		if (2006 == error_no){
			write_debug_log("In Func[%s] : mysql_query() fail ! 数据库连接中断，重新启动服务!", funcName);
			exit(error_no);
		}
		write_debug_log("In Func[%s] : mysql_query() fail ! 错误码:%d, 详细信息:%s", funcName, error_no, errbuf);
		err = std::string(errbuf);
		free_conn(conn_no);
		return -5;
	}

	result = mysql_store_result(mysql_conn);
	iCount = mysql_num_rows(result);

	//班级没有学生
	if (iCount == 0){
		//组装JSON  
		try{
			for (auto cls : clsInfos_){
				json j_cls;
				json stuArray;
				j_cls[CLS_NAME] = cls.cls_name.c_str();
				j_cls[CLS_ID] = cls.cls_no.c_str();
				j_cls["students"] = json::parse(stuArray.dump().c_str());
				classInfoArray.push_back(j_cls);
			}
		}
		catch (...){
			write_debug_log("In Func[%s] : JSON组装异常", funcName);
			err = std::string("JSON组装异常!");
			free_conn(conn_no);
			return -6;
		}
		free_conn(conn_no);
		return 0;
	}

	row = mysql_fetch_row(result);
	if (NULL == result || NULL == row){
		write_debug_log("In Func[%s] : mysql_num_rows() fail ! iCount:%d, 用户不存在!", funcName, iCount);
		err = std::string("未查找到学生!");
		free_conn(conn_no);
		return -7;
	}

	//读取学生信息
	std::vector<student> students_;
	for (int i = 0; i < iCount; ++i){
		student stu;
		std::ostringstream itoa;
		itoa << row[3];
		stu.userId = row[0];
		stu.userName = row[1];
		stu.cls_no = itoa.str();
		students_.push_back(stu);
		row = mysql_fetch_row(result);
	}
	write_debug_log("In Func[%s] : 学生数据加载成功! 学生数:%d", funcName, iCount);

	//组装JSON  
	try{
		for (auto cls : clsInfos_){
			json j_cls;
			json stuArray;
			for (auto it : students_){
				if (it.cls_no == cls.cls_no){
					json stu;
					it.to_json(stu);
					stuArray.push_back(stu);
				}
			}
			j_cls[CLS_NAME] = cls.cls_name.c_str();
			j_cls[CLS_ID] = cls.cls_no.c_str();
			j_cls["students"] = json::parse(stuArray.dump().c_str());
			classInfoArray.push_back(j_cls);
		}
	}
	catch (...){
		write_debug_log("In Func[%s] : JSON组装异常", funcName);
		err = std::string("JSON组装异常!");
		free_conn(conn_no);
		return -8;
	}

	free_conn(conn_no);
	return 0;
}

int queryClassInfo(CommThreadInfo* thread_info, unsigned char* data)
{
	int iRet;
	json jret;
	json clsInfoArray;
	std::string str_data((char*)data);
	std::string t_id;
	std::string str_ret;
	std::ostringstream oss_debug;
	const char* funcName = "queryClassInfo";

	write_debug_log("In Func[%s] : 接收到查询班级请求报文, 包体内容:%s", funcName, data);

	try{
		// parse json
		json j1 = json::parse(str_data);
		t_id = j1.at(PERSON_ID).get<std::string>();
		oss_debug << "t_id:" << t_id << '\n';
		write_debug_log("In Func[%s] : Json 解析成功:%s", funcName, oss_debug.str().c_str());

		// dispose data
		std::string str_err;
		iRet = queryClassInfoSqlOpt(t_id, clsInfoArray, str_err);
		if (0 != iRet){
			jret["result"] = "1";
			jret["msg"] = str_err.c_str();
			str_ret = jret.dump();
			write_debug_log("In Func[%s] : queryClassInfoSqlOpt() fail ! iRet:%d, str_err:%s", funcName, iRet, str_err.c_str());
		}
		else{
			jret["result"] = "0";
			jret["msg"] = "success";
			jret["classInfo"] = json::parse(clsInfoArray.dump().c_str());
			str_ret = jret.dump();
			write_debug_log("In Func[%s] : queryClassInfoSqlOpt() success!", funcName);
		}
	}// end try
	catch (...){
		jret["result"] = "1";
		jret["msg"] = "JSON处理异常!";
		str_ret = jret.dump();
		write_debug_log("In Func[%s] : JSON处理异常!", funcName);
		iRet = send_packet(thread_info->comm_sock, CLIENT_REQ_QUERY_CLASS, str_ret.length(), (const unsigned char*)str_ret.c_str());
		if (iRet <= 0){
			write_debug_log("In Func[%s] : 发送应答包错误! iRet:%d, sys_err:%d", funcName, iRet, errno);
		}
		return iRet;
	}//end catch	

	iRet = send_packet(thread_info->comm_sock, CLIENT_REQ_QUERY_CLASS, str_ret.length(), (const unsigned char*)str_ret.c_str());
	if (iRet <= 0){
		write_debug_log("In Func[%s] : 发送应答包错误! iRet:%d, sys_err:%d", funcName, iRet, errno);
	}
	write_debug_log("In Func[%s] : 发送应答包成功! 发送总长:%d, 包体长:%d, 包体内容:%s", funcName, iRet, str_ret.length(), str_ret.c_str());
	return iRet;
}

//5.查询班级 End

//6.创建活动 Begin
int insertActSqlOpt(int conn_no, ActivityReq& actReq, std::string& err)
{
	int iRet;
	MYSQL* mysql_conn = g_conn_list[conn_no].dbc_p;
	std::ostringstream sql_stmt;
	char errbuf[SQL_ERR_BUF_SIZE];
	int error_no;
	const char* funcName = "insertActSqlOpt";

	if (NULL == g_conn_list || conn_no < 0 || conn_no >= g_conn_count) {
		err = std::string("无效数据");
		write_debug_log("In Func[%s] 参数检查不通过!", funcName);
		return ERRNO_ILLEGAL_PARAM;
	}

	sql_stmt << "INSERT INTO ACTIVITY_TBL(act_name, t_id, act_class) VALUES('";

	sql_stmt << actReq.actName << "','" << actReq.userId << "','";
	for (auto it : actReq.classes_)
		sql_stmt << it << ",";
	sql_stmt << "');";
	write_debug_log("In Func[%s] : SQL:%s", funcName, sql_stmt.str().c_str());

	iRet = mysql_query(mysql_conn, sql_stmt.str().c_str());
	if (0 != iRet){
		get_conn_error(&g_conn_list[conn_no], &error_no, errbuf, SQL_ERR_BUF_SIZE);
		if (2006 == error_no){
			write_debug_log("In Func[%s] : execute_stmt() fail ! 数据库连接中断，重新启动服务!", funcName);
			exit(error_no);
		}
		write_debug_log("In Func[%s] : execute_stmt() fail ! 错误码:%d, 详细信息:%s", funcName, error_no, errbuf);
		err = std::string(errbuf);
		return -1;
	}

	std::ostringstream itoa;
	unsigned long last_id = mysql_insert_id(mysql_conn);
	itoa << last_id;
	actReq.actNo = itoa.str();

	//生成学生签到表
	// 1.查询活动涉及班级的所有学生:
	sql_stmt.str("");
	sql_stmt << "SELECT * FROM STUDENT_TBL WHERE cls_no in(";

	auto it = actReq.classes_.begin();
	auto itEnd = actReq.classes_.end();
	for (; it != itEnd - 1; ++it)
		sql_stmt << *it << ",";
	sql_stmt << *it << ")";

	write_debug_log("In Func[%s] : SQL:%s", funcName, sql_stmt.str().c_str());

	iRet = mysql_refresh(mysql_conn, REFRESH_TABLES);
	if (0 != iRet){
		get_conn_error(&g_conn_list[conn_no], &error_no, errbuf, SQL_ERR_BUF_SIZE);
		if (2006 == error_no){
			write_debug_log("In Func[%s] : mysql_refresh() fail ! 数据库连接中断，重新启动服务!", funcName);
			exit(error_no);
		}
		write_debug_log("In Func[%s] : mysql_refresh() fail ! 错误码:%d, 详细信息:%s", funcName, error_no, errbuf);
		err = std::string(errbuf);
		return -2;
	}

	iRet = mysql_query(mysql_conn, sql_stmt.str().c_str());
	if (0 != iRet){
		get_conn_error(&g_conn_list[conn_no], &error_no, errbuf, SQL_ERR_BUF_SIZE);
		if (2006 == error_no){
			write_debug_log("In Func[%s] : mysql_query() fail ! 数据库连接中断，重新启动服务!", funcName);
			exit(error_no);
		}
		write_debug_log("In Func[%s] : mysql_query() fail ! 错误码:%d, 详细信息:%s", funcName, error_no, errbuf);
		err = std::string(errbuf);
		return -3;
	}

	MYSQL_RES* result = mysql_store_result(mysql_conn);
	int iCount = mysql_num_rows(result);
	MYSQL_ROW row = mysql_fetch_row(result);

	//没有学生
	if (iCount == 0){
		write_debug_log("In Func[%s] : [WARNING]该活动班级没有学生信息!", funcName, iCount);
		return 0;
	}


	if (NULL == result|| NULL == row){
		write_debug_log("In Func[%s] : mysql_num_rows() fail ! iCount:%d, 获取学生信息错误!", funcName, iCount);
		err = std::string("获取学生信息错误!");
		return -4;
	}


	std::vector<ActivitySignInfo> signs_;
	for (int i = 0; i < iCount; ++i){
		ActivitySignInfo sign;
		sign.actNo = actReq.actNo;
		sign.actName = actReq.actName;
		sign.userId.assign(row[0]);//学生号
		sign.userName.assign(row[1]);//学生名
		sign.c_id.assign(row[3]);//班级号
		sign.c_Name.assign(row[4]);//班级号
		signs_.push_back(sign);
		row = mysql_fetch_row(result);
	}
	mysql_free_result(result);
	write_debug_log("In Func[%s] : 加载学生信息成功! 学生数:%d", funcName, iCount);

	// 2.遍历学生信息 并 插入到签到表
	//mysql_autocommit(mysql_conn, 0);
	for (auto it : signs_){
		std::ostringstream ossBatchInert;
		ossBatchInert << "INSERT INTO SIGNIN_TBL(act_no,act_name, s_id,s_name, cls_no, cls_name, sign_status)VALUES("
			<< actReq.actNo << ",'"
			<< actReq.actName << "','"
			<< it.userId << "','"//学生号
			<<it.userName << "',"//学生名
			<< it.c_id << ",'"//班级号
			<< it.c_Name << "','"//班级名
			<< "0');";
		//iRet = mysql_query(mysql_conn, ossInert.str().c_str());
		iRet = mysql_query(mysql_conn, ossBatchInert.str().c_str());
		if (0 != iRet){
			get_conn_error(&g_conn_list[conn_no], &error_no, errbuf, SQL_ERR_BUF_SIZE);
			if (2006 == error_no){
				write_debug_log("In Func[%s] : mysql_query() fail ! 数据库连接中断，重新启动服务!", funcName);
				exit(error_no);
			}
			write_debug_log("In Func[%s] : mysql_query() fail ! SQL:%s, 错误码:%d, 详细信息:%s", funcName, ossBatchInert.str().c_str(), error_no, errbuf);
			err = std::string(errbuf);
			return -5;
		}
		write_debug_log("In Func[%s] : SQL insert OK:\n%s", funcName, ossBatchInert.str().c_str());
	}
	//mysql_commit(mysql_conn);
	/*std::string rm_last_comma = ossBatchInert.str();
	size_t pos = rm_last_comma.rfind(',');
	rm_last_comma.replace(pos, 1, ";");*/

	return 0;
}

int insertActivity(ActivityReq& actReq, std::string& err)
{
	const char* funcName = "insertActivity";

	if (0 == actReq.userId.size() || 0 == actReq.actName.size())  {
		err = std::string("无效数据");
		write_debug_log("In Func[%s] 参数检查不通过!", funcName);
		return ERRNO_ILLEGAL_PARAM;
	}

	int conn_no = get_free_conn();
	if (conn_no < 0){
		err = std::string("无可用连接");
		write_debug_log("In Func[%s] : get_free_conn() fail", funcName);
		return -1;
	}

	int iRet = insertActSqlOpt(conn_no, actReq, err);
	if (iRet != 0){
		rollback_trans(conn_no);
		free_conn(conn_no);
		write_debug_log("In Func[%s] : insertActSqlOpt() fail, iRet:%d", funcName, iRet);
		return -2;
	}

	commit_trans(conn_no);
	free_conn(conn_no);
	return 0;
}

int addActivity(CommThreadInfo* thread_info, unsigned char* data)
{
	int iRet;
	json jret;
	ActivityReq actReq;
	std::string str_data((char*)data);
	std::string str_ret;
	std::ostringstream oss_debug;
	const char* funcName = "addActivity";

	write_debug_log("In Func[%s] : 接收到添加活动请求报文, 包体内容:%s", funcName, data);

	try{
		////去掉斜杠
		//for (auto it = str_data.begin(); it != str_data.end();){
		//	if (*it == '\\')
		//		it = str_data.erase(it);
		//	++it;
		//}

		// parse json
		json j1 = json::parse(str_data);
		json j_arry;

		actReq.userId = j1[PERSON_ID]; 
		actReq.actName = j1[ACT_NAME];
		oss_debug << "userId:" << actReq.userId << '\n'
			<< "actName:" << actReq.actName << '\n';

		j_arry = j1[CLS_ID];
		for (auto it : j_arry)
			actReq.classes_.push_back(it);
		
		for (unsigned int i = 0; i < actReq.classes_.size(); ++i)
			oss_debug << "classes_[" << i << "]:" << actReq.classes_[i] << '\n';
		
		write_debug_log("In Func[%s] : Json 解析成功:%s", funcName, oss_debug.str().c_str());

		// dispose data
		std::string str_err;
		iRet = insertActivity(actReq, str_err);
		if (0 != iRet){
			jret["result"] = "1";
			jret["msg"] = str_err.c_str();
			str_ret = jret.dump();
			write_debug_log("In Func[%s] : insertActivity() fail ! iRet:%d", funcName, iRet);
		}
		else{
			jret["result"] = "0";
			jret["msg"] = "success";
			jret[ACT_NO] = actReq.actNo;
			str_ret = jret.dump();
			write_debug_log("In Func[%s] : insertActivity() success!", funcName);
		}
	}// end try
	catch (...){
		jret["result"] = "1";
		jret["msg"] = "JSON处理异常!";
		str_ret = jret.dump();
		write_debug_log("In Func[%s] : JSON处理异常!", funcName);
		iRet = send_packet(thread_info->comm_sock, CLIENT_REQ_ADD_ACTIVITY, str_ret.length(), (const unsigned char*)str_ret.c_str());
		if (iRet <= 0){
			write_debug_log("In Func[%s] : 发送应答包错误! iRet:%d, sys_err:%d", funcName, iRet, errno);
		}
		return iRet;
	}//end catch	

	iRet = send_packet(thread_info->comm_sock, CLIENT_REQ_ADD_ACTIVITY, str_ret.length(), (const unsigned char*)str_ret.c_str());
	if (iRet <= 0){
		write_debug_log("In Func[%s] : 发送应答包错误! iRet:%d, sys_err:%d", funcName, iRet, errno);
	}
	write_debug_log("In Func[%s] : 发送应答包成功! 发送总长:%d, 包体长:%d, 包体内容:%s", funcName, iRet, str_ret.length(), str_ret.c_str());
	return iRet;
}

//6.创建活动 End

//7.活动签到 Begin
int updateSignInfoSqlOpt(int conn_no, const ActivitySignInfo& signInfo, std::string& err)
{
	int iRet;
	MYSQL* mysql_conn = g_conn_list[conn_no].dbc_p;
	std::ostringstream sql_stmt;
	char errbuf[SQL_ERR_BUF_SIZE];
	int error_no;
	const char* funcName = "updateSignInfoSqlOpt";

	if (NULL == g_conn_list || conn_no < 0 || conn_no >= g_conn_count)	return -1;

	sql_stmt << "UPDATE SIGNIN_TBL SET sign_date=curdate(), sign_time=curtime(), sign_status='1' WHERE act_no ="
		<< signInfo.actNo << " and s_id = '"
		<< signInfo.userId << "';";
	write_debug_log("In Func[%s] : SQL:%s", funcName, sql_stmt.str().c_str());

	iRet = mysql_query(mysql_conn, sql_stmt.str().c_str());
	if (0 != iRet){
		get_conn_error(&g_conn_list[conn_no], &error_no, errbuf, SQL_ERR_BUF_SIZE);
		if (2006 == error_no){
			write_debug_log("In Func[%s] : mysql_query() fail ! 数据库连接中断，重新启动服务!", funcName);
			exit(error_no);
		}
		write_debug_log("In Func[%s] : mysql_query() fail ! 错误码:%d, 详细信息:%s", funcName, error_no, errbuf);
		err = std::string(errbuf);
		return -4;
	}

	return 0;
}

int updateSignInfo(const ActivitySignInfo& signInfo, std::string& err)
{
	const char* funcName = "updateSignInfo";

	if (0 == signInfo.userId.size() || 0 == signInfo.actNo.size())  {
		err = std::string("无效数据");
		write_debug_log("In Func[%s] 参数检查不通过!", funcName);
		return ERRNO_ILLEGAL_PARAM;
	}

	int conn_no = get_free_conn();
	if (conn_no < 0){
		err = std::string("无可用连接");
		write_debug_log("In Func[%s] : get_free_conn() fail", funcName);
		return -1;
	}

	int iRet = updateSignInfoSqlOpt(conn_no, signInfo, err);
	if (iRet != 0){
		rollback_trans(conn_no);
		free_conn(conn_no);
		write_debug_log("In Func[%s] : updateStuSqlOpt() fail, iRet:%d", funcName, iRet);
		return -2;
	}

	commit_trans(conn_no);
	free_conn(conn_no);
	return 0;
}

int stuSignIn(CommThreadInfo* thread_info, unsigned char* data)
{
	int iRet;
	json jret;
	ActivitySignInfo signInfo;
	std::string str_data((char*)data);
	std::string str_ret;
	std::ostringstream oss_debug;
	const char* funcName = "stuSignIn";

	write_debug_log("In Func[%s] : 接收到学生签到请求报文, 包体内容:%s", funcName, data);

	try{
		// parse json
		json j1 = json::parse(str_data);
		json j_arry;

		signInfo.userId = j1[PERSON_ID];
		signInfo.actNo = j1[ACT_NO];
		oss_debug << "userId:" << signInfo.userId << '\n' << "actNo:" << signInfo.actNo << '\n';
		write_debug_log("In Func[%s] : Json 解析成功:%s", funcName, oss_debug.str().c_str());

		// dispose data
		std::string str_err;
		iRet = updateSignInfo(signInfo, str_err);
		if (0 != iRet){
			jret["result"] = "1";
			jret["msg"] = str_err.c_str();
			str_ret = jret.dump();
			write_debug_log("In Func[%s] : insertActivity() fail ! iRet:%d", funcName, iRet);
		}
		else{
			jret["result"] = "0";
			jret["msg"] = "success";
			str_ret = jret.dump();
			write_debug_log("In Func[%s] : insertActivity() success!", funcName);
		}
	}// end try
	catch (...){
		jret["result"] = "1";
		jret["msg"] = "JSON处理异常!";
		str_ret = jret.dump();
		write_debug_log("In Func[%s] : JSON处理异常!", funcName);
		iRet = send_packet(thread_info->comm_sock, CLIENT_REQ_JOIN_ACTIVITY, str_ret.length(), (const unsigned char*)str_ret.c_str());
		if (iRet <= 0){
			write_debug_log("In Func[%s] : 发送应答包错误! iRet:%d, sys_err:%d", funcName, iRet, errno);
		}
		return iRet;
	}//end catch	

	iRet = send_packet(thread_info->comm_sock, CLIENT_REQ_JOIN_ACTIVITY, str_ret.length(), (const unsigned char*)str_ret.c_str());
	if (iRet <= 0){
		write_debug_log("In Func[%s] : 发送应答包错误! iRet:%d, sys_err:%d", funcName, iRet, errno);
	}
	write_debug_log("In Func[%s] : 发送应答包成功! 发送总长:%d, 包体长:%d, 包体内容:%s", funcName, iRet, str_ret.length(), str_ret.c_str());
	return iRet;
}
//7.活动签到 End

//8.查询活动 Begin
int queryActInfoSqlOpt(const std::string& t_id, json& actInfoArray, std::string& err)
{
	int iRet;
	std::ostringstream sql_stmt;
	char errbuf[SQL_ERR_BUF_SIZE];
	int error_no;
	const char* funcName = "queryActInfoSqlOpt";

	if (0 == t_id.size()) {
		err = std::string("无效数据");
		write_debug_log("In Func[%s] 参数检查不通过!", funcName);
		return ERRNO_ILLEGAL_PARAM;
	}

	int conn_no = get_free_conn();
	if (conn_no < 0 || NULL == g_conn_list || conn_no >= g_conn_count){
		err = std::string("无可用连接");
		write_debug_log("In Func[%s] : get_free_conn() fail", funcName);
		return -1;
	}

	MYSQL* mysql_conn = g_conn_list[conn_no].dbc_p;
	iRet = mysql_refresh(mysql_conn, REFRESH_TABLES);
	if (0 != iRet){
		get_conn_error(&g_conn_list[conn_no], &error_no, errbuf, SQL_ERR_BUF_SIZE);
		if (2006 == error_no){
			write_debug_log("In Func[%s] : mysql_refresh() fail ! 数据库连接中断，重新启动服务!", funcName);
			exit(error_no);
		}
		write_debug_log("In Func[%s] : mysql_refresh() fail ! 错误码:%d, 详细信息:%s", funcName, error_no, errbuf);
		err = std::string(errbuf);
		free_conn(conn_no);
		return -2;
	}

	sql_stmt << "SELECT * FROM ACTIVITY_TBL WHERE t_id = '" << t_id << "';";
	write_debug_log("In Func[%s] : SQL:%s", funcName, sql_stmt.str().c_str());

	iRet = mysql_query(mysql_conn, sql_stmt.str().c_str());
	if (0 != iRet){
		get_conn_error(&g_conn_list[conn_no], &error_no, errbuf, SQL_ERR_BUF_SIZE);
		if (2006 == error_no){
			write_debug_log("In Func[%s] : mysql_query() fail ! 数据库连接中断，重新启动服务!", funcName);
			exit(error_no);
		}
		write_debug_log("In Func[%s] : mysql_query() fail ! 错误码:%d, 详细信息:%s", funcName, error_no, errbuf);
		err = std::string(errbuf);
		free_conn(conn_no);
		return -3;
	}

	MYSQL_RES* result = mysql_store_result(mysql_conn);
	int iCount = mysql_num_rows(result);
	MYSQL_ROW row = mysql_fetch_row(result);

	//没有活动信息
	if (iCount == 0){
		write_debug_log("In Func[%s] : 该教师[%s]未创建活动", funcName, t_id.c_str());
		free_conn(conn_no);
		return 0;
	}

	if (NULL == result || NULL == row){
		write_debug_log("In Func[%s] : 获取活动信息失败!", funcName, t_id.c_str());
		err = std::string("获取活动信息失败!");
		free_conn(conn_no);
		return -7;
	}

	//读取活动信息
	std::vector<ActivitySignInfo> acts_;
	for (int i = 0; i < iCount; ++i){
		ActivitySignInfo tmp;
		std::ostringstream itoa;
		itoa << row[0];//活动号
		tmp.actNo = itoa.str();
		tmp.actName = row[1];
		acts_.push_back(tmp);
		row = mysql_fetch_row(result);
	}
	write_debug_log("In Func[%s] : 活动信息加载成功! 活动数:%d", funcName, iCount);

	//组装JSON   
	try{
		for (auto it : acts_){
			json act;
			act[ACT_NO] = it.actNo;
			act[ACT_NAME] = it.actName;
			actInfoArray.push_back(act);
		}
	}
	catch (...){
		write_debug_log("In Func[%s] : JSON组装异常", funcName);
		err = std::string("JSON组装异常!");
		free_conn(conn_no);
		return -8;
	}

	free_conn(conn_no);
	return 0;
}

int queryActivity(CommThreadInfo* thread_info, unsigned char* data)
{
	int iRet;
	json jret;
	json actInfoArray;
	std::string str_data((char*)data);
	std::string t_id;
	std::string str_ret;
	std::ostringstream oss_debug;
	const char* funcName = "queryActivity";

	write_debug_log("In Func[%s] : 收到查询活动请求报文, 包体内容:%s", funcName, data);

	try{
		// parse json
		json j1 = json::parse(str_data);
		t_id = j1.at(PERSON_ID).get<std::string>();
		oss_debug << "t_id:" << t_id << '\n';
		write_debug_log("In Func[%s] : Json 解析成功:%s", funcName, oss_debug.str().c_str());

		// dispose data
		std::string str_err;
		iRet = queryActInfoSqlOpt(t_id, actInfoArray, str_err);
		if (0 != iRet){
			jret["result"] = "1";
			jret["msg"] = str_err.c_str();
			str_ret = jret.dump();
			write_debug_log("In Func[%s] : queryClassInfoSqlOpt() fail ! iRet:%d, str_err:%s", funcName, iRet, str_err.c_str());
		}
		else{
			jret["result"] = "0";
			jret["msg"] = "success";
			jret["activityInfo"] = json::parse(actInfoArray.dump().c_str());
			str_ret = jret.dump();
			write_debug_log("In Func[%s] : queryClassInfoSqlOpt() success!", funcName);
		}
	}// end try
	catch (...){
		jret["result"] = "1";
		jret["msg"] = "JSON处理异常!";
		str_ret = jret.dump();
		write_debug_log("In Func[%s] : JSON处理异常!", funcName);
		iRet = send_packet(thread_info->comm_sock, CLIENT_REQ_QUERY_ACTIVITY, str_ret.length(), (const unsigned char*)str_ret.c_str());
		if (iRet <= 0){
			write_debug_log("In Func[%s] : 发送应答包错误! iRet:%d, sys_err:%d", funcName, iRet, errno);
		}
		return iRet;
	}//end catch	

	iRet = send_packet(thread_info->comm_sock, CLIENT_REQ_QUERY_ACTIVITY, str_ret.length(), (const unsigned char*)str_ret.c_str());
	if (iRet <= 0){
		write_debug_log("In Func[%s] : 发送应答包错误! iRet:%d, sys_err:%d", funcName, iRet, errno);
	}
	write_debug_log("In Func[%s] : 发送应答包成功! 发送总长:%d, 包体长:%d, 包体内容:%s", funcName, iRet, str_ret.length(), str_ret.c_str());
	return iRet;
}
//8.查询活动 End

//9.查询签到
int querySignInfoSqlOpt(const ActivityReq& actInfo, json& signInfoArray, std::string& err)
{
	int iRet;
	std::ostringstream sql_stmt;
	char errbuf[SQL_ERR_BUF_SIZE];
	int error_no;
	const char* funcName = "querySignInfoSqlOpt";

	if (0 == actInfo.userId.size() || 0 == actInfo.actNo.size()) {
		err = std::string("无效数据");
		write_debug_log("In Func[%s] 参数检查不通过!", funcName);
		return ERRNO_ILLEGAL_PARAM;
	}

	int conn_no = get_free_conn();
	if (conn_no < 0 || NULL == g_conn_list || conn_no >= g_conn_count){
		err = std::string("无可用连接");
		write_debug_log("In Func[%s] : get_free_conn() fail", funcName);
		return -1;
	}

	MYSQL* mysql_conn = g_conn_list[conn_no].dbc_p;
	iRet = mysql_refresh(mysql_conn, REFRESH_TABLES);
	if (0 != iRet){
		get_conn_error(&g_conn_list[conn_no], &error_no, errbuf, SQL_ERR_BUF_SIZE);
		if (2006 == error_no){
			write_debug_log("In Func[%s] : mysql_refresh() fail ! 数据库连接中断，重新启动服务!", funcName);
			exit(error_no);
		}
		write_debug_log("In Func[%s] : mysql_refresh() fail ! 错误码:%d, 详细信息:%s", funcName, error_no, errbuf);
		err = std::string(errbuf);
		free_conn(conn_no);
		return -2;
	}

	sql_stmt << "SELECT * FROM SIGNIN_TBL WHERE act_no = " << actInfo.actNo 
		<< " ORDER BY cls_no, sign_status ASC;";
	write_debug_log("In Func[%s] : SQL:%s", funcName, sql_stmt.str().c_str());

	iRet = mysql_query(mysql_conn, sql_stmt.str().c_str());
	if (0 != iRet){
		get_conn_error(&g_conn_list[conn_no], &error_no, errbuf, SQL_ERR_BUF_SIZE);
		if (2006 == error_no){
			write_debug_log("In Func[%s] : mysql_query() fail ! 数据库连接中断，重新启动服务!", funcName);
			exit(error_no);
		}
		write_debug_log("In Func[%s] : mysql_query() fail ! 错误码:%d, 详细信息:%s", funcName, error_no, errbuf);
		err = std::string(errbuf);
		free_conn(conn_no);
		return -3;
	}

	MYSQL_RES* result = mysql_store_result(mysql_conn);
	int iCount = mysql_num_rows(result);
	MYSQL_ROW row = mysql_fetch_row(result);

	//没有签到信息
	if (iCount == 0){
		write_debug_log("In Func[%s] : 未查询到签到信息, 活动号:%s", funcName, actInfo.actNo.c_str());
		free_conn(conn_no);
		return 0;
	}

	if (NULL == result || NULL == row){
		write_debug_log("In Func[%s] : 获取签到信息失败! 活动号:%s", funcName, actInfo.actNo.c_str());
		err = std::string("获取活动信息失败!");
		free_conn(conn_no);
		return -7;
	}

	write_debug_log("In Func[%s] : 查询签到信息成功! 签到数:%d, 开始加载...", funcName, iCount);
	//读取签到信息
	std::vector<ActivitySignInfo> signs_;
	for (int i = 0; i < iCount; ++i){
		ActivitySignInfo tmp;
		std::ostringstream itoa;
		itoa << row[0];//活动号
		tmp.actNo = itoa.str();
		tmp.actName = row[1];
		tmp.userId = row[2];//学生号
		tmp.userName = row[3];//学生名
		if (row[4])	tmp.c_id = row[4];
		if (row[5])	tmp.c_Name = row[5];
		if (row[6])	tmp.sign_date = row[6];
		if (row[7])	tmp.sign_time = row[7];
		if (row[8])	tmp.sign_status = row[8];
		//write_debug_log("In Func[%s] : push_back()", funcName);
		signs_.push_back(tmp);
		row = mysql_fetch_row(result);
	}
	write_debug_log("In Func[%s] : 签到信息加载成功! 活动数:%d", funcName, iCount);

	//组装JSON   
	try{
		for (auto it : signs_){
			json sign;
			sign[ACT_NO] = it.actNo;
			sign[ACT_NAME] = it.actName;
			sign[PERSON_ID] = it.userId;
			sign[PERSON_NAME] = it.userName;
			sign[CLS_ID] = it.c_id;
			sign[CLS_NAME] = it.c_Name;
			sign["status"] = it.sign_status;
			sign["date"] = it.sign_date;
			sign["time"] = it.sign_time;

			signInfoArray.push_back(sign);
		}
	}
	catch (...){
		write_debug_log("In Func[%s] : JSON组装异常", funcName);
		err = std::string("JSON组装异常!");
		free_conn(conn_no);
		return -8;
	}

	free_conn(conn_no);
	return 0;
}

int querySignInfo(CommThreadInfo* thread_info, unsigned char* data)
{
	int iRet;
	json jret;
	std::string str_data((char*)data);
	json signInfoArray;
	ActivityReq actInfo;
	std::string str_ret;
	std::ostringstream oss_debug;
	const char* funcName = "querySignInfo";

	write_debug_log("In Func[%s] : 收到查询签到请求报文, 包体内容:%s", funcName, data);

	try{
		// parse json
		json j1 = json::parse(str_data);
		actInfo.userId = j1.at(PERSON_ID).get<std::string>();
		actInfo.actNo = j1.at(ACT_NO).get<std::string>();

		oss_debug << "t_id:" << actInfo.userId << '\n'
			<< "actNo:" << actInfo.actNo << '\n';
		write_debug_log("In Func[%s] : Json 解析成功:%s", funcName, oss_debug.str().c_str());

		// dispose data
		std::string str_err;
		iRet = querySignInfoSqlOpt(actInfo, signInfoArray, str_err);
		if (0 != iRet){
			jret["result"] = "1";
			jret["msg"] = str_err.c_str();
			str_ret = jret.dump();
			write_debug_log("In Func[%s] : queryClassInfoSqlOpt() fail ! iRet:%d, str_err:%s", funcName, iRet, str_err.c_str());
		}
		else{
			jret["result"] = "0";
			jret["msg"] = "success";
			jret["signInfo"] = json::parse(signInfoArray.dump().c_str());
			str_ret = jret.dump();
			write_debug_log("In Func[%s] : queryClassInfoSqlOpt() success!", funcName);
		}
	}// end try
	catch (...){
		jret["result"] = "1";
		jret["msg"] = "JSON处理异常!";
		str_ret = jret.dump();
		write_debug_log("In Func[%s] : JSON处理异常!", funcName);
		iRet = send_packet(thread_info->comm_sock, CLIENT_REQ_QUERY_SIGN, str_ret.length(), (const unsigned char*)str_ret.c_str());
		if (iRet <= 0){
			write_debug_log("In Func[%s] : 发送应答包错误! iRet:%d, sys_err:%d", funcName, iRet, errno);
		}
		return iRet;
	}//end catch	

	iRet = send_packet(thread_info->comm_sock, CLIENT_REQ_QUERY_SIGN, str_ret.length(), (const unsigned char*)str_ret.c_str());
	if (iRet <= 0){
		write_debug_log("In Func[%s] : 发送应答包错误! iRet:%d, sys_err:%d", funcName, iRet, errno);
	}
	write_debug_log("In Func[%s] : 发送应答包成功! 发送总长:%d, 包体长:%d, 包体内容:%s", funcName, iRet, str_ret.length(), str_ret.c_str());
	return iRet;
}
