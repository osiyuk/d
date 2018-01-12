
enum {
  PARSE_OK,
  SYNTAX_ERROR,
  NEGATIVE_ID,
  STRING_OVERFLOW
};

uint8_t parse_insert(const char * buffer, table_row_t * row);

