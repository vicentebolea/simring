#include <mysql.h>

#if defined CPLUSPLUS
extern "C" { 
#endif 

MYSQL* conn;

bool mysql_setup (char* url, char* user, char* pass, char* db) {
 conn = mysql_init (NULL); 
 int ret = mysql_real_connect (conn, url, user, pass, db, 0, NULL, 0);

 if (ret != 0) return false;

 return true;
}

bool mysql_log (char* table, char* key, char* value) {
 char query [128];
 sprintf (query, "INSERT INTO %s (%s) VALUES (\'%s\');", table, key, value);
 mysql_query (conn, query);
 return true;
}


#if defined CPLUSPLUS
}
#endif 
