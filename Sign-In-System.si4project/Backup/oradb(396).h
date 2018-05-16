
#ifndef ORADB_H

#define ORADB_H

#include <mysql/mysql.h>

#define  SQL_ERR_BUF_SIZE 1024

typedef struct tagOraConn{
	MYSQL* dbc_p;
	int free_flag; 
}OraConn;

typedef struct tagOraStmt{
	OraConn* ora_conn_p;
	MYSQL_STMT* stmt_p;
	unsigned long param_count;
	MYSQL_BIND* bind_param_list;
}OraStmt;

typedef struct tagField{
	char field_name[31];
	char* field_buf;
	unsigned long field_size;
	unsigned long field_length;
}Field;

typedef struct tagOraQuery{
	OraStmt ora_stmt;
	int field_count;
	MYSQL_BIND* bind_field_list;
	Field* field_list;
}OraQuery;

int init_db_env();
void uninit_db_env();

int open_db_conn(char* userid, char* password,
		char* server, int dbport, char* dbname,
		OraConn* ora_conn);
void close_db_conn(OraConn* ora_conn);
int db_conn_opened(OraConn* ora_conn);
int commit(OraConn* ora_conn);
int rollback(OraConn* ora_conn);

int prepare_stmt(OraConn* ora_conn, char* sql_stmt_text, 
		OraStmt* ora_stmt);
int execute_stmt(OraStmt* ora_stmt);
int free_stmt(OraStmt* ora_stmt);
int bind_by_pos(OraStmt* ora_stmt, unsigned  int pos, char* value);

int prepare_query(OraConn* ora_conn, char* sql_text,
		OraQuery* ora_query);
int open_query(OraQuery* ora_query);
int close_query(OraQuery* ora_query);
int bind_query_by_pos(OraQuery* ora_query, unsigned  int pos, char* value);
int get_field_by_name(OraQuery* ora_query, char* name, 
		char* value, int size);
int get_field_by_pos(OraQuery* ora_query, int pos, 
		char* value, int size);
int move_to_first(OraQuery* ora_query);
int move_to_next(OraQuery* ora_query);
int move_to_prior(OraQuery* ora_query);
int move_to_last(OraQuery* ora_query);

void get_conn_error(OraConn* ora_conn,
		int* sql_code, char* message, int buf_len);
void get_query_error(OraQuery* ora_query,
		int* sql_code, char* message, int buf_len);
void get_stmt_error(OraStmt* ora_stmt,
		int* sql_code, char* message, int buf_len);

#endif


