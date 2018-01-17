/*
 * Copyright (c) 2017-2018 Kiril Osiyuk <kiril.saradomin@gmail.com>
 * Bits and pieces stolen from Joern Engel code, which is
 * Copyright (c) 2007-2008 Joern Engel <joern@logfs.org>
 *
 * A relatively complex B+Tree implementation. I have written it
 * as learning exercise to understand how B+Trees work.
 *
 * DOC: B+Tree (variant of B-Tree) basics
 *
 * A B+Tree is a data structure for looking up arbitrary u32 keys
 * into u32 page numbers. This data structure is described at
 * http://scholarpedia.org/article/B-tree_and_UB-tree
 *
 * Each B+Tree consists of a head, that contains bookkeeping info
 * and a variable number (starting with zero) of nodes.
 * Each node contains the keys and page numbers, or,
 * for leaf nodes, the keys and associated variable length values.
 *
 * Each leaf in this implementation has following layout:
 * [page_num] [key1, key2, ..., keyN] [val1, val2, ...,valN]
 *
 * Each value here is binary data of geo->vlen bytes in total.
 * The number of keys and values (N) is geo->no_pairs.
 */

#define BUG(condition) do { if (condition) \
  printf("BUG! failure at %s:%d/%s\n", \
  __FILE__, __LINE__, __func__); } while (0)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
typedef uint32_t u32;

#if 1
/*
 * pager interface for mm
 */
struct pager_head;

void *pager_get(struct pager_head *, u32 index);
void *pager_next(struct pager_head *);
void pager_flush(struct pager_head *, u32 index, u32 size);

#endif

#define PAGESIZE 4096
#define MINIMUM_NO_LINKS(klen) (PAGESIZE - 2 * klen) / (4 * klen)
#define NO_PAIRS(klen, vlen) (PAGESIZE - klen) / (klen + vlen)

/*
 * btree geometry
 * @k: minimum number of keys in node (except root)
 * @vlen: length of value (in bytes)
 * @no_pairs: number of keys and values in leaf node
 */
struct btree_geo {
  int k;
  int vlen;
  int no_pairs;
};


struct btree_head {
  void *node;
  void *mempool;
  int height;
};


void *btree_geo(int vlen)
{
  struct btree_geo * geo;
  int klen = sizeof(u32);
  
  BUG(vlen < 1 || vlen > PAGESIZE);
  if (vlen < 1 || vlen > PAGESIZE) return NULL;
  
  geo = malloc(sizeof(struct btree_geo));
  if (!geo) return NULL;
  
  geo->k = MINIMUM_NO_LINKS(klen);
  geo->vlen = vlen;
  geo->no_pairs = NO_PAIRS(klen, vlen);
  
  return geo;
}


void *btree_alloc(struct pager_head * mempool)
{
  struct btree_head * head;
  
  BUG(!mempool);
  
  head = malloc(sizeof(struct btree_head));
  if (!head) return NULL;
  
  head->node = NULL;
  head->mempool = mempool;
  head->height = 0;
  
  return head;
}


void btree_init(struct btree_head * head)
{
  u32 * node;
  
  node = pager_get(head->mempool, 0);
  BUG(!node);
  
  head->node = node;
  head->height = node[0]; /* trick */
}


static u32 pgno(void * node)
{
  u32 * head = node;
  
  return *head;
}


static u32 bkey(struct btree_geo * geo, void * node, int pos)
{
  u32 * keys = (u32 *) node + 1;
  
  BUG(!node || pos < 0 || pos >= geo->no_pairs);
  
  return keys[pos];
}


static void setkey(struct btree_geo * geo,
                   void * node, int pos, u32 key)
{
  u32 * keys = (u32 *) node + 1;
  
  BUG(!node || pos < 0 || pos >= geo->no_pairs);
  
  keys[pos] = key;
}


static void * bval(struct btree_geo * geo, void * node, int pos)
{
  void * val;
  
  BUG(!node || pos < 0 || pos >= geo->no_pairs);
  
  val = (u32 *) node + (1 + geo->no_pairs);
  return (char *) val + (pos * geo->vlen);
}


static void setval(struct btree_geo * geo,
                   void * node, int pos, void * value)
{
  memcpy(bval(geo, node, pos), value, geo->vlen);
}


static int keycmp(struct btree_geo * geo,
                  void * node, int pos, u32 key)
{
  if (bkey(geo, node, pos) < key) return -1;
  if (bkey(geo, node, pos) > key) return 1;
  return 0;
}


static int getpos(struct btree_geo * geo, void * node, u32 key)
{
  int i;
  
  for (i = 0; i < geo->no_pairs; i++) {
    if (bkey(geo, node, i) == 0) break;
    if (keycmp(geo, node, i, key) >= 0) break;
  }
  
  return i;
}


static int getfill(struct btree_geo * geo, void * node, int start)
{
  int i;
  
  for (i = start; i < geo->no_pairs; i++) {
    if (bkey(geo, node, i) == 0) break;
  }
  
  return i;
}


/*
 * Usually this function is quite similar to normal insert.
 * But leaf and internal nodes should be treated differently.
 */
int btree_insert_level(struct btree_head * head,
                       struct btree_geo * geo,
                       u32 key, void * val, int level);


int btree_insert(struct btree_head * head,
                 struct btree_geo * geo, u32 key, void * val)
{
  void * node;
  int i, pos, fill;
  
  BUG(!val);
  
  node = head->node;
  pos = getpos(geo, node, key);
  fill = getfill(geo, node, pos);
  
  /* two identical keys are not allowed */
  BUG(pos < fill && keycmp(geo, node, pos, key) == 0);
  
  if (fill < geo->no_pairs) goto insert;
  
  /* need to split leaf */
  return 1; /* fill == geo->no_pairs */
  
insert:
  /* shift and insert */
  for (i = fill; i > pos; i--) {
    setkey(geo, node, i, bkey(geo, node, i - 1));
    setval(geo, node, i, bval(geo, node, i - 1));
  }
  setkey(geo, node, pos, key);
  setval(geo, node, pos, val);
  
  pager_flush(head->mempool, pgno(node), PAGESIZE);
  return 0;
}


void * btree_lookup(struct btree_head * head,
                    struct btree_geo * geo, u32 key)
{
  int i;
  void * node = head->node;
  
  for (i = 0; i < geo->no_pairs; i++) {
    if (keycmp(geo, node, i, key) == 0)
      return bval(geo, node, i);
  }
  
  return NULL;
}


/*
 * used in select.c to access all values
 */
void btree_iterate(struct btree_head * head,
                   struct btree_geo * geo,
                   void ** val, int * count)
{
  *val = bval(geo, head->node, 0);
  *count = getfill(geo, head->node, 0);
}


void btree_dump(struct btree_head * head,
                struct btree_geo * geo)
{
  void * node;
  int i;
  
#define RED "\x1b[31m"
#define BLUE "\x1b[34m"
#define GREEN "\x1b[32m"

#define NL "\n"
#define DIGIT "%u"
#define DEFAULT "\x1b[39m"
  
  printf(RED "k=" DIGIT DEFAULT NL, geo->k);
  printf("maximum keys=" DIGIT NL, 2 * geo->k);
  printf("value length=" DIGIT NL, geo->vlen);
  printf("no_pairs=" DIGIT, geo->no_pairs);
  printf(" (values per one page)" NL);
  printf("height=" DIGIT NL NL, head->height);
  
  node = head->node;
  
  printf(GREEN "leaf node %p" DEFAULT NL, node);
  if (!node) return;
  
  printf("keys=[" DIGIT, bkey(geo, node, 0));
  for (i = 1; i < geo->no_pairs; i++) {
    if (bkey(geo, node, i) == 0) break;
    printf(", " DIGIT, bkey(geo, node, i));
  }
  printf("]" NL);
}


