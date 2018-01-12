#include <string.h>
#include <stdlib.h>

#include "schema.h"
#include "parser.h"


typedef uint8_t u8;
typedef uint32_t u32;


static u8 parse_string(char ** saveptr, size_t length)
{
  char * str = strtok(NULL, " ");
  if (str == NULL) return SYNTAX_ERROR;
  
  if (strlen(str) > length) return STRING_OVERFLOW;
  
  *saveptr = str;
  return PARSE_OK;
}


u8 parse_insert(const char * buffer, table_row_t * row)
{
  char *id_string, *name, *email;
  int id;
  u8 r;
#define IS_PARSEOK \
  if (r != PARSE_OK) return r;
  
  strtok( (char *) buffer, " ");
  id_string = strtok(NULL, " ");
  if (id_string == NULL) return SYNTAX_ERROR;
  
  id = atoi(id_string);
  if (id < 0) return NEGATIVE_ID;
  
  r = parse_string(&name, USERNAME_SIZE);
  IS_PARSEOK
  
  r = parse_string(&email, EMAIL_SIZE);
  IS_PARSEOK
  
  row->id = id;
  strcpy(row->username, name);
  strcpy(row->email, email);
  
  return PARSE_OK;
}

