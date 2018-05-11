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

//request code
#define CLIENT_REQ_REGISTER 		 1000 //register    
#define CLIENT_REQ_LOG_IN   		 1001 //log in 	
									 
#define CLIENT_REQ_ADD_CLASS		 1002 
#define CLIENT_REQ_QUERY_CLASS		 1003 
#define CLIENT_REQ_JOIN_CLASS		 1004 
									 
#define CLIENT_REQ_ADD_ACTIVITY		 1005 
#define CLIENT_REQ_JOIN_ACTIVITY	 1006 
#define CLIENT_REQ_QUERY_ACTIVITY	 1007 

//error code
#define ERRNO_ILLEGAL_PARAM		-1000


//tag
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
	std::string t_id; //�����༶�Ľ�ʦID
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
}stuAndClsMap;

/*For Query cls Begin*/
typedef struct student{
	void to_json(json& j) {
		j = {
			{ "userId", userId },
			{ "UserName", userName }
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
	std::string userId;
	std::string actName;
	std::string actNo;
	std::vector<std::string> classes_;
}ActivityReq;

typedef struct ActivityStuInfo{
	std::string userId;
	std::string userName;
	std::string c_id;
	std::string c_Name;
}ActivityStuInfo;

typedef struct ActivitySignInfo{
	std::string actNo;
	std::string actName;
	std::string userId;
	std::string userName;
	std::string c_id;
	std::string c_Name;
	std::string sign_status;
	std::string sign_date;
	std::string sign_time;
}ActivitySignInfo;
/*For Add Activity End*/


/*Func*/
//ע��
int insertTeachOrStu(int conn_no, const person& person, std::string& err);
int insertPerson(const person& person, std::string& err);
int userRegister(CommThreadInfo* thread_info, unsigned char* data);

//��¼
int queryPerson(person& person, std::string& err);
int userSignUp(CommThreadInfo* thread_info, unsigned char* data);

//��Ӱ༶
int insertClsSqlOpt(int conn_no, const classInfo& cls, std::string& err);
int insertCls(const classInfo& cls, std::string& err);
int addClassRequest(CommThreadInfo* thread_info, unsigned char* data);

//����༶
int updateStuSqlOpt(int conn_no, const stuAndClsMap& mapInfo, std::string& err);
int updateStudent(const stuAndClsMap& mapInfo, std::string& err);
int joinClassRequest(CommThreadInfo* thread_info, unsigned char* data);

//��ѯ�༶
int queryClassInfoSqlOpt(const std::string& t_id, json& classInfoArray, std::string& err);
int queryClassInfo(CommThreadInfo* thread_info, unsigned char* data);

//�����
int insertActSqlOpt(int conn_no, ActivityReq& actReq, std::string& err);
int insertActivity(ActivityReq& actReq, std::string& err);
int addActivity(CommThreadInfo* thread_info, unsigned char* data);







#endif

