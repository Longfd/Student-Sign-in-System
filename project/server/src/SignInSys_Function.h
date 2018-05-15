/*
文件名称: SignInSys_Function.h
文件说明: 签到所有请求类型定义及各类型处理函数
包括: 
	1. 签到请求类型定义
	2. 各类型处理函数
*/

/*
 *Description: Interface for disposing Sign-in-System Client's Message
 *Author: 	  LongFeida
 *Date: 	  2018-05-01
 *Version: 	  v1.0.0.1
 *Authority:  XianYu Technology Co.,Ltd
 *Revison:	  
 */

#ifndef __SignInSys_Function_H__
#define __SignInSys_Function_H__

//请求消息码定义
#define CLIENT_REQ_REGISTER 		 1000 //register    
#define CLIENT_REQ_LOG_IN   		 1001 //log in 	
									 
#define CLIENT_REQ_ADD_CLASS		 1002 
#define CLIENT_REQ_QUERY_CLASS		 1003 
#define CLIENT_REQ_JOIN_CLASS		 1004 
									 
#define CLIENT_REQ_ADD_ACTIVITY		 1005 
#define CLIENT_REQ_JOIN_ACTIVITY	 1006 
#define CLIENT_REQ_QUERY_ACTIVITY	 1007 
#define CLIENT_REQ_QUERY_SIGN		 1008

//错误码定义
#define ERRNO_ILLEGAL_PARAM		-1000


//JSON标签定义
#define RESPON_CODE "result"
#define RESPON_MSG "msg"
#define PERSON_ID "userId"
#define PERSON_NAME "userName"
#define PERSON_ROLE "role"
#define PERSON_PWD "pwd"

#define CLS_ID "cls_no"
#define CLS_NAME "cls_name"
#define ACT_NAME "act_name"
#define ACT_NO "act_no"

#include <string>
#include <vector>

#include "json.hpp"
using json = nlohmann::json;

typedef struct tagCommThreadInfo CommThreadInfo;

typedef struct person{
	void to_json(json& j) {
		j = json{ { PERSON_ID, userId },
		{ PERSON_NAME, userName },
		{ PERSON_ROLE, role },
		{ PERSON_PWD, pwd } };
	}
	void from_json(const json& j) {
		userId = j.at(PERSON_ID).get<std::string>();
		userName = j.at(PERSON_NAME).get<std::string>();
		role = j.at(PERSON_ROLE).get<std::string>();
		pwd = j.at(PERSON_PWD).get<std::string>();
	}

	std::string userId;
	std::string userName;
	std::string role;
	std::string pwd;
	//for student:
	std::string cls_id;
	std::string cls_name;
}person;

typedef struct classInfo{
	void to_json(json& j) {
		j = json{
		//{ CLS_ID, clsId },
			{ CLS_NAME, clsName },
			{ PERSON_ID, t_id }
		};
	}
	void from_json(const json& j) {
		//clsId = j.at(CLS_ID).get<std::string>();
		clsName = j.at(CLS_NAME).get<std::string>();
		t_id = j.at(PERSON_ID).get<std::string>();
	}
	//std::string clsId;
	std::string clsName;
	std::string t_id;
}classInfo;

typedef struct stuAndClsMap{
	void to_json(json& j) {
		j = json{
		//{ CLS_ID, clsId },
			{ PERSON_ID, s_id },
			{ CLS_ID, cls_no }
	};
	}
	void from_json(const json& j) {
		//clsId = j.at(CLS_ID).get<std::string>();
		s_id = j.at(PERSON_ID).get<std::string>();
		cls_no = j.at(CLS_ID).get<std::string>();
	}
	std::string s_id;
	std::string cls_no;
	std::string cls_name;
}stuAndClsMap;

/*For Query cls Begin*/
typedef struct student{
	void to_json(json& j) {
		j = {
			{ "userId", userId },
			{ "userName", userName }
		};
	}
	void from_json(const json& j){
		userId = j.at("userId").get<std::string>();
		userName = j.at("userName").get<std::string>();
	}
	std::string userId;
	std::string userName;
	std::string cls_no;
}student;

typedef struct classWithStu{
	std::string cls_no;
	std::string cls_name;
	std::vector<student> students_;
}classWithStu;
/*For Query cls End*/

/*For Add Activity Begin*/
typedef struct ActivityReq{
	std::string actName;
	std::string actNo;
	std::string userId;
	std::vector<std::string> classes_;
}ActivityReq;

typedef struct ActivityStuInfo{
	std::string userId;//教师号
	std::string userName;//教师名
	std::string c_id;
	std::string c_Name;
}ActivityStuInfo;

typedef struct ActivitySignInfo{
	std::string actNo;
	std::string actName;
	std::string userId;//学生号
	std::string userName;//学生名
	std::string c_id;
	std::string c_Name;
	std::string sign_status;
	std::string sign_date;
	std::string sign_time;
}ActivitySignInfo;
/*For Add Activity End*/


/*Func*/
//教师或学生注册
int insertTeachOrStu(int conn_no, const person& person, std::string& err);
int insertPerson(const person& person, std::string& err);
int userRegister(CommThreadInfo* thread_info, unsigned char* data);

//教师或学生登录
int queryPerson(person& person, std::string& err);
int userSignUp(CommThreadInfo* thread_info, unsigned char* data);

//教师添加班级
int insertClsSqlOpt(int conn_no, const classInfo& cls, std::string& err);
int insertCls(const classInfo& cls, std::string& err);
int addClassRequest(CommThreadInfo* thread_info, unsigned char* data);

//学生加入班级༶
int updateStuSqlOpt(int conn_no, stuAndClsMap& mapInfo, std::string& err);
int updateStudent(stuAndClsMap& mapInfo, std::string& err);
int joinClassRequest(CommThreadInfo* thread_info, unsigned char* data);

//教师查询班级
int queryClassInfoSqlOpt(const std::string& t_id, json& classInfoArray, std::string& err);
int queryClassInfo(CommThreadInfo* thread_info, unsigned char* data);

//教师创建活动
int insertActSqlOpt(int conn_no, ActivityReq& actReq, std::string& err);
int insertActivity(ActivityReq& actReq, std::string& err);
int addActivity(CommThreadInfo* thread_info, unsigned char* data);

//学生活动签到
int updateSignInfoSqlOpt(int conn_no, const ActivitySignInfo& signInfo, std::string& err);
int updateSignInfo(const ActivitySignInfo& signInfo, std::string& err);
int stuSignIn(CommThreadInfo* thread_info, unsigned char* data);

//教师查询活动
int queryActInfoSqlOpt(const std::string& t_id, json& actInfoArray, std::string& err);
int queryActivity(CommThreadInfo* thread_info, unsigned char* data);

//教师查询签到
int querySignInfoSqlOpt(const ActivityReq& actInfo, json& signInfoArray, std::string& err);
int querySignInfo(CommThreadInfo* thread_info, unsigned char* data);

#endif

