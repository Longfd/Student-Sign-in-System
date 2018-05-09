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

#define CLIENT_REQ_REGISTER 	 1000 //register    
#define CLIENT_REQ_LOG_IN   	 1001 //log in 	

#define CLIENT_REQ_ADD_CLASS	 1002 
#define CLIENT_REQ_QUERY_CLASS	 1003 

#define CLIENT_REQ_ADD_COURSE	 1004 
#define CLIENT_REQ_SLCT_COURSE	 1005 
#define CLIENT_REQ_QUERY_COURSE	 1006 //student(query course has selected) 
									  //teacher(query course has created)

#define CLIENT_REQ_ADD_SIGN	 	 1007 //sign in	


#define RESPON_CODE "result"
#define RESPON_MSG "msg"
#define PERSON_ID "userId"
#define PERSON_NAME "userName"
#define PERSON_ROLE "role"
#define PERSON_PWD "pwd"

#define CLS_ID "cls_no"
#define CLS_NAME "cls_name"

#include <string>
#include <vector>

#include "/home/user1/github/json/single_include/nlohmann/json.hpp"
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
	std::string t_id; //创建班级的教师ID
}classInfo;

/*Func*/
int insertTeachOrStu(int conn_no, const person& person, std::string& err);
int insertPerson(const person& person, std::string& err);
int userRegister(CommThreadInfo* thread_info, unsigned char* data);

int queryPerson(person& person, std::string& err);
int userSignUp(CommThreadInfo* thread_info, unsigned char* data);

int insertClsSqlOpt(int conn_no, const classInfo& cls, std::string& err);
int insertCls(const classInfo& cls, std::string& err);
int addClassRequest(CommThreadInfo* thread_info, unsigned char* data);













#endif

