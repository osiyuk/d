#include <stdint.h>

/* Start small by putting a lot of limitations:
 * support only two operations
 * reside in memory
 * support a single, hard-coded table
 
CREATE TABLE users (
  id INT,
  username VARCHAR(32),
  email VARCHAR(255)
);

 * This is simple schema, but it gets us to support
 * multiple data types and multiple text sizes.
 */

enum {
  USERNAME_SIZE = 32,
  EMAIL_SIZE = 255,
};

typedef struct {
  uint32_t id;
  char username[USERNAME_SIZE + 1];
  char email[EMAIL_SIZE + 1];
} table_row_t;

enum {
  INSERT,
  SELECT
};

typedef struct {
  uint8_t type;
  table_row_t * row;
} statement_t;

