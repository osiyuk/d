#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "schema.h"
#include "parser.h"
#include "b+tree.h"

void *pager_open(const char * filename);

void *btree, *geometry;


void db_open(const char * filename)
{
  void * pager = pager_open(filename);
  
  btree = btree_alloc(pager);
  btree_init(btree);
  
  if (!btree) {
    puts("db_open: btree not initialized");
    exit(EXIT_FAILURE);
  }
  geometry = btree_geo(sizeof(table_row_t));
}


void read_input(char ** input)
{
  static size_t length;
  static char * line;
  ssize_t n;
  
  n = getline(&line, &length, stdin);
  if (n <= 0) {
    perror("read_input");
    exit(EXIT_FAILURE);
  }
  
  /* ignoring trailing newline */
  if (line[n - 1] == '\n') line[n - 1] = 0;
  
  *input = line;
}


void sql_meta_command(const char *command) {
  if (strcmp(command, ".exit") == 0) {
    exit(EXIT_SUCCESS);
  }
  if (strcmp(command, ".btree") == 0) {
    btree_dump(btree, geometry);
    return;
  }
  
  printf("Unrecognized command '%s'.\n", command);
}


void sql_execute_statement(statement_t * statement) {
  uint32_t key;
  int key_exists;
  
  switch (statement->type) {
  case INSERT:
    key = statement->row->id;
    
    key_exists = (btree_lookup(btree, geometry, key) != NULL);
    if (key_exists) goto key_duplicate;
    
    if (btree_insert(btree, geometry, key, statement->row)) {
      puts("Need to implement splitting a leaf node.");
      return;
    }
    break;
    
  case SELECT:
    btree_print_values(btree, geometry);
    break;
  }
  puts("Executed.");
  return;

key_duplicate:
  puts("Error: duplicate key.");
  return;
}


void sql_command_processor(char * input) {
  statement_t statement;
  table_row_t row;
  uint8_t result;
  
  if (input[0] == '.') {
    sql_meta_command(input);
    return;
  }
  
  if (strncmp(input, "insert", 6) == 0) {
    statement.type = INSERT;
    statement.row = &row;
    
    result = parse_insert(input, &row);
    
    switch (result) {
    case PARSE_OK: goto execute;
    
    case SYNTAX_ERROR:
      puts("Syntax error. Could not parse statement.");
      puts("Usage: insert id name email");
      return;
    
    case NEGATIVE_ID:
      puts("Error: id must be positive.");
      return;
    
    case STRING_OVERFLOW:
      puts("Error: string is too long.");
      return;
    
    default:
      puts("sql_command_processor: insert failure");
      exit(EXIT_FAILURE);
    }
  }
  if (strcmp(input, "select") == 0) {
    statement.type = SELECT;
    
    goto execute;
  }
  
  printf("Unrecognized keyword at start of '%s'.\n", input);
  return;
  
execute:
  sql_execute_statement(&statement);
}


int main(int argc, char *argv[])
{
  char * input;
  
  if (argc < 2) {
    puts("Must supply a database filename.");
    exit(EXIT_FAILURE);
  }
  db_open(argv[1]);
  
  
  while (1) {
    printf("db > ");
    read_input(&input);
    sql_command_processor(input);
  }
}

