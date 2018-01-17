#ifndef BTREE_H
#define BTREE_H

/* bookkeeping information */
struct btree_head;

/* btree geometry */
struct btree_geo;

/*
 * allocate btree_head with given mempool (must not be NULL)
 * currently we use pager.c for mm
 * returns btree_head
 */
void *btree_alloc(void * mempool);

/*
 * initialise a btree
 * loads root node and sets bookeeping information
 */
void btree_init(struct btree_head * head);

/*
 * calculate geometry
 * @value_length should be between 1 and PAGESIZE
 * returns btree_geo, or NULL if it failed
 */
void *btree_geo(int value_length);

/*
 * returns the value for the given key, or NULL
 */
void *btree_lookup(struct btree_head *,
                   struct btree_geo *, u32 key);

/*
 * returns 0 if the value inserted, or an
 * error code if it failed (may fail due to memory pressure).
 */
int btree_insert(struct btree_head *,
                 struct btree_geo *, u32 key, void * value);

/*
 * currently used in SELECT statement
 * see also select.c
 */
void btree_print_values(struct btree_head *,
                        struct btree_geo *);

/*
 * used for development
 */
void btree_dump(struct btree_head *,
                struct btree_geo *);

#endif

