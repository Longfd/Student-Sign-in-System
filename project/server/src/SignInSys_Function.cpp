#include <sstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "SignInSys_Function.h"
#include "SignInSys_db.h"
#include "oradb.h"
#include "packet.h"
#include "loginfo.h"
#include "comm_thread.h"


extern int g_conn_count;
extern OraConn* g_conn_list;





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
	write_debug_log("In Func[%s] : 用户验证不通过! pwd[%s], pwdDB[%s]", funcName, person.pwd, row[2]);
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

	if (NULL == g_conn_list || conn_no < 0 || conn_no >= g_conn_count)	return -1;

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
int updateStuSqlOpt(int conn_no, const stuAndClsMap& mapInfo, std::string& err)
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

int updateStudent(const stuAndClsMap& mapInfo, std::string& err)
{
	const char* funcName = "updateStudent";

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
