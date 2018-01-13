/*
 * Start small by putting a lot of limitations:
 - reside in memory
 * support only two operations
 * support a single, hard-coded table

CREATE TABLE users (
  id INT,
  username VARCHAR(32),
  email VARCHAR(255)
);

 * This is simple schema, but it gets us to support
 * multiple data types and multiple text sizes.
 */

#include <stdint.h>
typedef uint8_t u8;
typedef uint32_t u32;

enum {
  USERNAME_SIZE = 32,
  EMAIL_SIZE = 255,
};

typedef struct {
  u32 id;
  char username[USERNAME_SIZE + 1];
  char email[EMAIL_SIZE + 1];
} table_row_t;

enum {
  INSERT,
  SELECT
};

typedef struct {
  u8 type;
  table_row_t * row;
} statement_t;

