/*
 * Oversimplified page/disk/buffer cache
 * I have written it as learning exercise
 * to understand how page cache work.
 *
 * If you want guts https://en.wikipedia.org/wiki/Page_cache
 *
 * TODO: study mmap, munmap <sys/mman.h>
 */

#include <fcntl.h> /* open */
#include <unistd.h> /* lseek, read, write */
#include <stdlib.h>
#include <stdio.h>

#include <stdint.h>
typedef uint32_t u32;

enum {
  MAX_PAGES = 100,
  PAGESIZE = 4096,
};

struct pager_head {
  int fd;
  u32 no_pages;
  void *cache[MAX_PAGES];
};


void *pager_open(const char * filename)
{
  struct pager_head * pager;
  int fd, no_pages;
  off_t file_length;
  
  pager = malloc(sizeof(struct pager_head));
  if (pager == NULL) goto error;
  
  fd = open(filename, O_RDWR | O_CREAT, 0600);
  if (fd == -1) goto error;
  
  file_length = lseek(fd, 0, SEEK_END);
  if (file_length == -1) goto error;
  
  no_pages = (file_length / PAGESIZE);
  if (file_length % PAGESIZE) no_pages++;
  
  pager->fd = fd;
  pager->no_pages = no_pages;
  
  return pager;
  
error:
  perror("pager_open");
  exit(EXIT_FAILURE);
}


void *pager_get(struct pager_head * pager, u32 index)
{
  void * page;
  ssize_t err;
  
  if (index > MAX_PAGES) goto boundary;
  
  page = pager->cache[index];
  if (page) return page;
  
  /* cache miss */
  page = malloc(PAGESIZE);
  if (page == NULL) goto error;
  
  if (index > pager->no_pages) goto success;
  
  /* found on disk */
  err = lseek(pager->fd, (index * PAGESIZE), SEEK_SET);
  if (err == -1) goto error;
  
  err = read(pager->fd, page, PAGESIZE);
  if (err == -1) goto error;
  
  goto success;
  
boundary:
  printf("pager_get: tried to fetch out of bound"
    ", %d > %d\n", index, MAX_PAGES);
  exit(EXIT_FAILURE);
  
error:
  perror("pager_get");
  exit(EXIT_FAILURE);
  
success:
  pager->cache[index] = page;
  return page;
}


void *pager_next(struct pager_head * head)
{
  /* BUG: pager can allocate ahead of no_pages */
  return pager_get(head, head->no_pages++);
}


void pager_flush(struct pager_head * pager, u32 index, u32 size)
{
  void * page;
  ssize_t err;
  
  page = pager->cache[index];
  if (page == NULL) goto hole;
  
  err = lseek(pager->fd, (index * PAGESIZE), SEEK_SET);
  if (err == -1) goto error;
  
  err = write(pager->fd, page, size);
  if (err == -1) goto error;
  
  return;
  
hole:
  puts("pager_flush: tried to flush null page");
  exit(EXIT_FAILURE);
  
error:
  perror("pager_flush");
  exit(EXIT_FAILURE);
}

