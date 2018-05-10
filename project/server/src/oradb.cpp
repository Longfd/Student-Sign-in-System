#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#include "oradb.h"

void get_conn_error(OraConn* ora_conn, 
		int* sql_code, char* message, int buf_len)
{
	*sql_code = mysql_errno(ora_conn->dbc_p);
	strncpy(message, mysql_error(ora_conn->dbc_p), buf_len-1);
	message[buf_len-1] = 0;
}

void get_query_error(OraQuery* ora_query,
		int* sql_code, char* message, int buf_len)
{
	get_stmt_error(&ora_query->ora_stmt, 
			sql_code, message, buf_len);
}

void get_stmt_error(OraStmt* ora_stmt,
		int* sql_code, char* message, int buf_len)
{
	*sql_code = mysql_stmt_errno(ora_stmt->stmt_p);
	strncpy(message, 
			mysql_stmt_error(ora_stmt->stmt_p), buf_len-1);
}

int init_db_env()
{
	return 0;
}

void uninit_db_env()
{
}

int open_db_conn(char* userid, char* password, 
		char* server, int dbport, char* dbname, 
		OraConn* ora_conn)
{
	ora_conn->dbc_p = mysql_init(NULL);
	if (NULL == ora_conn->dbc_p)
	{
		return 1;
	}

	mysql_options(ora_conn->dbc_p, 
			MYSQL_SET_CHARSET_NAME, "utf8");

	if (mysql_real_connect(ora_conn->dbc_p,
				server, userid, password, dbname, dbport,
				NULL, 0) == NULL)
	{
		return 1;
	}

	mysql_autocommit(ora_conn->dbc_p, 0);
	return 0;
}

void close_db_conn(OraConn* ora_conn)
{
	if (NULL == ora_conn)
	{
		return;
	}

	if(ora_conn->dbc_p !=NULL)
	{
		mysql_close(ora_conn->dbc_p);
		ora_conn->dbc_p = NULL;
	}
}

int commit(OraConn* ora_conn)
{
	int ret_code;

	if (NULL == ora_conn)
	{
		return -1;
	}

	if(NULL == ora_conn->dbc_p)
	{
		return -2;
	}

	/* 执行commit */
	ret_code = mysql_commit(ora_conn->dbc_p);
	if (ret_code != 0)
	{
		return 1; 
	}

	return 0;
}

int rollback(OraConn* ora_conn)
{
	int ret_code;

	if (NULL == ora_conn)
	{
		return -1;
	}

	if(NULL == ora_conn->dbc_p)
	{
		return -2;
	}

	/* 执行rollback */
	ret_code = mysql_rollback(ora_conn->dbc_p);
	if (ret_code != 0)
	{
		return 1;
	}

	return 0;
}

void build_stmt_param(OraStmt* ora_stmt)
{
	unsigned long param_count;

	param_count = mysql_stmt_param_count(ora_stmt->stmt_p);
	if (param_count == 0)
	{
		ora_stmt->param_count = 0;
		ora_stmt->bind_param_list = NULL;
		return;
	}

	ora_stmt->param_count = param_count;
	ora_stmt->bind_param_list = (MYSQL_BIND*)malloc(
			param_count*sizeof(MYSQL_BIND));
	memset(ora_stmt->bind_param_list, 0,
			param_count*sizeof(MYSQL_BIND));
}

int prepare_stmt(OraConn* ora_conn, char* sql_text, 
		OraStmt* ora_stmt)
{
	int ret_code;
	if (NULL == ora_conn)
	{
		return -1;
	}

	if (NULL == ora_conn->dbc_p)
	{
		return -2;
	}

	if (NULL == sql_text)
	{
		return -3;
	}

	if (NULL == ora_stmt)
	{
		return -4;
	}

	ora_stmt->ora_conn_p = ora_conn;
	ora_stmt->stmt_p = mysql_stmt_init(
			ora_conn->dbc_p);
	if (NULL == ora_stmt->stmt_p)
	{
		return 1;
	} 

	/*  执行prepare */
	ret_code = mysql_stmt_prepare(ora_stmt->stmt_p, 
			sql_text, strlen(sql_text));
	if (ret_code != 0)
	{
		return 1; /* 有错误,可以取错误信息 */
	}

	build_stmt_param(ora_stmt);  
	return 0;
}

int execute_sql_stmt(OraStmt* ora_stmt)
{
	int ret_code;

	if (NULL == ora_stmt)
	{
		return -1;
	}

	if (NULL == ora_stmt->ora_conn_p)
	{
		return -2;
	}

	if(NULL == ora_stmt->stmt_p)
	{
		return -3;
	}

	if (ora_stmt->param_count > 0)
	{
		ret_code = mysql_stmt_bind_param(
				ora_stmt->stmt_p, ora_stmt->bind_param_list);
		if (ret_code != 0)
		{
			return 1;
		}
	}
	/* 执行语句 */
	ret_code = mysql_stmt_execute(ora_stmt->stmt_p);
	if (ret_code != 0)
	{
		return 1;
	}

	return 0;
}

int execute_stmt(OraStmt* ora_stmt)
{
	return execute_sql_stmt(ora_stmt);
}

int free_stmt(OraStmt* ora_stmt)
{
	if (NULL == ora_stmt)
	{
		return -1;
	}

	if (NULL != ora_stmt->stmt_p)
	{
		mysql_stmt_close(ora_stmt->stmt_p);
		ora_stmt->stmt_p = NULL;
	}

	if (NULL != ora_stmt->bind_param_list)
	{
		free(ora_stmt->bind_param_list);
		ora_stmt->bind_param_list = NULL;
	}
	return 0;
}

int bind_by_pos(OraStmt* ora_stmt, unsigned int position, char* value)
{
	MYSQL_BIND* param;

	char tmp[1024] = {0};
	strcpy(tmp, value);

	if (NULL == ora_stmt)
	{
		return -1;
	}

	if (NULL == ora_stmt->ora_conn_p)
	{
		return -2;
	}

	if(NULL == ora_stmt->stmt_p)
	{
		return -3;
	}

	if (position<1 || position>ora_stmt->param_count)
	{
		return -4;
	}

	param = &ora_stmt->bind_param_list[position-1];
	param->buffer_type = MYSQL_TYPE_STRING;
	param->buffer = tmp;
	param->buffer_length = strlen(tmp);
	return 0;
}

int bind_blob_by_pos(OraStmt* ora_stmt, unsigned int position, 
		char* value, int length)
{
	MYSQL_BIND* param;

	if (NULL == ora_stmt)
	{
		return -1;
	}

	if (NULL == ora_stmt->ora_conn_p)
	{
		return -2;
	}

	if(NULL == ora_stmt->stmt_p)
	{
		return -3;
	}

	if (position<1 || position>ora_stmt->param_count)
	{
		return -4;
	}
	param = &ora_stmt->bind_param_list[position-1];
	param->buffer_type = MYSQL_TYPE_BLOB;
	param->buffer = value;
	param->buffer_length = length;
	return 0;
}

int bind_query_result(OraQuery* ora_query)
{
	MYSQL_RES* res;
	int field_count;
	int i, result;
	MYSQL_FIELD* field;

	res = mysql_stmt_result_metadata(
			ora_query->ora_stmt.stmt_p);
	if (res == NULL)
	{
		return 1;
	}

	field_count = mysql_num_fields(res);
	ora_query->field_count = field_count;
	ora_query->field_list 
		= (Field*)malloc(field_count*sizeof(Field));
	memset(ora_query->field_list, 0, 
			field_count*sizeof(Field));
	ora_query->bind_field_list = (MYSQL_BIND*)malloc(
			field_count*sizeof(MYSQL_BIND));
	memset(ora_query->bind_field_list, 0,
			field_count*sizeof(MYSQL_BIND));
	for (i=0; i<field_count; i++)
	{
		field = mysql_fetch_field_direct(res, i);
		ora_query->field_list[i].field_size = field->length;
		strcpy(ora_query->field_list[i].field_name, field->name);
		ora_query->field_list[i].field_length = 0;
		ora_query->field_list[i].field_buf 
			= (char*)malloc(field->length+1);

		ora_query->bind_field_list[i].buffer_type 
			= MYSQL_TYPE_STRING;
		ora_query->bind_field_list[i].buffer_length 
			= field->length;
		ora_query->bind_field_list[i].buffer 
			= ora_query->field_list[i].field_buf;
		ora_query->bind_field_list[i].length
			= &ora_query->field_list[i].field_length;
	}

	mysql_free_result(res);
	result = mysql_stmt_bind_result(
			ora_query->ora_stmt.stmt_p, 
			ora_query->bind_field_list);
	if (result != 0)
	{
		return 1;
	}
	return 0;
}

int prepare_query(OraConn* ora_conn, char* sql_text, 
		OraQuery* ora_query)
{
	int result;

	if (NULL == ora_query)
	{
		return -1;
	}

	result = prepare_stmt(ora_conn, sql_text, 
			&ora_query->ora_stmt);
	if (result != 0)
	{
		return result;
	}

	result = bind_query_result(ora_query);
	if (result != 0)
	{
		return 1;
	}

	return 0;
}

int open_query(OraQuery* ora_query)
{
	int ret_code;

	ret_code = execute_sql_stmt(&ora_query->ora_stmt);
	if (0 != ret_code)
	{
		return ret_code;
	}

	ret_code = mysql_stmt_store_result(
			ora_query->ora_stmt.stmt_p);
	if (ret_code != 0)
	{
		return 1;
	}

	return 0;
}

int move_to_first(OraQuery* ora_query)
{
	return -1; 
}

int move_to_next(OraQuery* ora_query)
{
	int i;
	Field* field;

	for (i=0; i<ora_query->field_count; i++)
	{
		field = &ora_query->field_list[i];
		memset(field->field_buf, 0, field->field_size+1);
	}
	return mysql_stmt_fetch(ora_query->ora_stmt.stmt_p);
}

int move_to_prior(OraQuery* ora_query)
{
	return -1;
}

int move_to_last(OraQuery* ora_query)
{
	return -1;
}

int close_query(OraQuery* ora_query)
{
	int i;

	if (NULL == ora_query)
	{
		return -1;
	}

	mysql_stmt_free_result(ora_query->ora_stmt.stmt_p); 
	free_stmt(&ora_query->ora_stmt);

	if (ora_query->field_list != NULL)
	{
		for (i=0; i<ora_query->field_count; i++)
		{
			free(ora_query->field_list[i].field_buf);
			ora_query->field_list[i].field_buf = NULL;
		}
		free(ora_query->field_list);
		ora_query->field_list = NULL;
	}

	if (ora_query->bind_field_list != NULL)
	{
		free(ora_query->bind_field_list);
		ora_query->bind_field_list = NULL;
	}
	return 0;
}

int bind_query_by_pos(OraQuery* ora_query, unsigned int position, char* value)
{
	return bind_by_pos(&ora_query->ora_stmt, position, value);
}

int get_field_by_pos(OraQuery* ora_query,
		int position, char* value, int size)
{
	Field* field;
	int field_count;

	if (NULL == ora_query)
	{
		return -1;
	}

	field_count = ora_query->field_count; 
	if (position<0 || position>=field_count)
	{
		return -2;
	}

	if (NULL == value)
	{
		return -3;
	}

	if (size < 2)
	{
		return -4;
	}

	field = &ora_query->field_list[position];
	strncpy(value, field->field_buf, size-1);
	value[size-1] = 0;

	return 0;
}

int get_field_by_name(OraQuery* ora_query, 
		char* name, char* value, int size)
{
	int i, field_count;
	Field* field;

	if (NULL == ora_query)
	{
		return -1;
	}

	if (NULL == name)
	{
		return -2;
	}

	if (NULL == value)
	{
		return -3;
	}

	if (size < 2)
	{
		return -4;
	}

	field_count = ora_query->field_count;
	for (i=0; i<field_count; i++)
	{
		field = &ora_query->field_list[i];
		if (strcmp(field->field_name, name) == 0)
		{
			strncpy(value, field->field_buf, size-1);
			value[size-1] = 0;
			return 0;
		}
	}

	return -5;
}


