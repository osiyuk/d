#include <stdio.h>
#include "schema.h"

#define FORMAT "(%u, %s, %s)\n"


struct btree_head;
struct btree_geo;


void btree_iterate(struct btree_head * head,
                   struct btree_geo * geo,
                   void ** val, int * count);


void btree_print_values(struct btree_head * head,
                        struct btree_geo * geo)
{
  table_row_t row, *rows;
  int i, count;
  
  btree_iterate(head, geo, (void **) &rows, &count);
  
  for (i = 0; i < count; i++) {
    row = rows[i];
    printf(FORMAT, row.id, row.username, row.email);
  }
}

