
/*
 * Jay Patel
 * jpatel71@hawk.iit.edu
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "mm.h"
#include "memlib.h"

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define BLK_HDR_SIZE (ALIGN(sizeof(blk_hdr)))

#define BLK_SIZE (ALIGN(BLK_HDR_SIZE + sizeof(size_t)))

typedef struct header blk_hdr;
//struct for header
struct header {
	size_t size;
	blk_hdr *next;
	blk_hdr *prev;
};

blk_hdr *coalesce(size_t *f);
blk_hdr *find_fit(size_t size);
void print_heap();
blk_hdr *f_list;
int i;
size_t b_size[] = {BLK_SIZE, BLK_SIZE + (1<<3), BLK_SIZE + (1<<5),
			BLK_SIZE + (1<<7), BLK_SIZE + (1<<8)};
size_t *LAST_REALLOCED = 0;

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
  f_list = mem_sbrk(5 * BLK_HDR_SIZE);
  for (i = 0; i<5; i++){
	f_list[i].next = f_list[i].prev = &f_list[i];
	f_list[i].size = BLK_HDR_SIZE;
	}	
  return 0;
}



/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
  int checksize = 0, shiftby = 2;
  while(checksize < size){
    checksize = 1<<shiftby++;
  }

  if (checksize - size > 200){
    checksize = size;
  }

  int newsize = ALIGN(BLK_HDR_SIZE + checksize + sizeof(size_t));

  blk_hdr * b = find_fit(newsize);

  if (b == NULL){
    b = mem_sbrk(newsize);
    if ((long)b == -1){
      return NULL;
    }

    b -> size = newsize | 1;
    size_t *ftr = (size_t *)((char *)b + ((b->size)&~1) - SIZE_T_SIZE);
    *ftr = newsize | 1;
    return (void *)((char *)b + SIZE_T_SIZE);
  }
  else {
    size_t m_blk = ((b->size)&~1) - newsize;
    if (m_blk > BLK_HDR_SIZE + SIZE_T_SIZE){
      b-> size = newsize | 1;
      size_t *n_ftr = (size_t *)((char *)b + ((b->size)&~1) - SIZE_T_SIZE);
      *n_ftr = b->size;

      blk_hdr *split = (blk_hdr *)((char *)b + ((b->size)&~1));
      split->size = m_blk&~1;

      size_t *ftr = (size_t *)(((char *)split + ((split->size)&~1)) - SIZE_T_SIZE);
      *ftr = m_blk&~1;

      blk_hdr *var = coalesce((size_t *)split);

      if(var != NULL)
        split = var;
      for (i = 4; i >= 0; i--) {
        if (b_size[i] <= split->size){
          split->next = f_list[i].next;
          split->prev = &f_list[i];
          f_list[i].next = split;
          split->next->prev = split;
          break;
        }
      }
    } 
    else {
      b->size = (b->size) | 1;
      size_t *ftr = (size_t *)(((char *)b + ((b->size)&~1)) - SIZE_T_SIZE);
      *ftr = (b->size) | 1;
    }
    return (void *)((char *)b + SIZE_T_SIZE);
    }
  }


  // void *p = mem_sbrk(newsize);
  // if ((long)p == -1)
  //   return NULL;
  // else {
  //   *(size_t *)p = size;
    //return (void *)((char *)p + SIZE_T_SIZE);



/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
  blk_hdr *b;
  blk_hdr *c_blk = coalesce((size_t *)((char *)ptr - SIZE_T_SIZE));

  if (c_blk == NULL){
    b = (blk_hdr *)((char *)ptr - SIZE_T_SIZE);
  }
  else{
    b = c_blk;
  }

  b->size = (b -> size) & ~1;

  for (i = 4; i >= 0; i--) {
    if (b_size[i] <= b->size) {
      size_t *ftr = (size_t *)(((char *)b + ((b->size)&~1)) - SIZE_T_SIZE);
      *ftr = (b->size) & ~1;
      
      b->next = f_list[i].next;
      b->prev = &f_list[i];
      f_list[i].next = b;
      b->next->prev = b;
      return;
    }
  }

}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
  if(size == 0){
    mm_free(ptr);
    return NULL;
  }
  else if (ptr == NULL)
    return mm_malloc(size);
  else{
    size_t *this_blk = (size_t *)((char *)ptr - SIZE_T_SIZE);
    int newsize = ALIGN(size + BLK_HDR_SIZE + sizeof(size_t));

    if (((*this_blk)&~1) >= newsize)
      return ptr;
    else if ((void *)((char *)this_blk + ((*this_blk)&~1)) == mem_heap_hi() + 1){
      newsize = ALIGN(size + ((*this_blk)&~1));
      mem_sbrk(ALIGN(size));
      *this_blk = newsize | 1;

      size_t *ftr = (size_t *)(((char *)this_blk + ((*this_blk)&~1) - SIZE_T_SIZE));
      *ftr = newsize | 1;
      return ptr;
    }
    else{
      size_t oldsize = (*this_blk)&~1;
      
      if (this_blk == LAST_REALLOCED)
        this_blk = mm_malloc(newsize * 2);
      else 
        this_blk = mm_malloc(newsize);

      memcpy(this_blk, ptr, oldsize - 2 * SIZE_T_SIZE);
      mm_free(ptr);
      size_t *t_blk = (size_t *)((char *)this_blk - SIZE_T_SIZE);
      LAST_REALLOCED = t_blk;
      return this_blk;
    }
  }
}

void print_heap(){
  blk_hdr *b = mem_heap_lo();
  while (b < (blk_hdr *)mem_heap_hi()){
    printf("%s BLOCK at %p, SIZE %d\n",
      (b->size&1)?"ALLOCATED": "FREE",b,(int)(b->size & ~1));
    b = (blk_hdr *)((char *)b + (b->size & ~1));
  } 
}

blk_hdr *find_fit(size_t size){
  blk_hdr *p;
  for(i = 0; i < 5; i++){
    if ((b_size[i] >= size && f_list[i].next != &f_list[i])){
      p = f_list[i].next;
      f_list[i].next = p->next;
      p->next->prev = &f_list[i];
      return p;
    }
  }
  
  p = f_list[4].next;
  while (p != &f_list[4]){
    if (p->size >= size){
      p->prev->next = p->next;
      p->next->prev = p->prev;
      return p;
    }
    p = p->next;
  }
  return NULL;
}

blk_hdr *coalesce(size_t *f)
{
  size_t *nxt = (size_t *)((char *)f + ((*f)&~1));
  size_t *prv = (size_t *)((char *)f - SIZE_T_SIZE);
  if ((void *)((char *)nxt - 1) == mem_heap_hi())
  {
    if (((char *)f - BLK_HDR_SIZE * 5) == mem_heap_lo())
      return NULL;
    else if ((*prv & 1) == 0)
    {
      blk_hdr *p_blk = (blk_hdr *)((char *)f - *prv);
      p_blk->size = p_blk->size + ((*f)&~1);
      p_blk->prev->next = p_blk->next;
      p_blk->next->prev = p_blk->prev;

      size_t *ftr = (size_t *)(((char *)p_blk + ((p_blk->size)&~1) - SIZE_T_SIZE));
      *ftr = (p_blk->size)&~1;
      return p_blk;
    }
    else
      return NULL;
  }
  else if (((char *)f - BLK_HDR_SIZE * 5) == mem_heap_lo())
  {
    if (((*nxt) & 1) == 0)
    {
      blk_hdr *n_blk = (blk_hdr *)((char *)f + ((*f)&~1));
      n_blk->prev->next = n_blk->next;
      n_blk->next->prev = n_blk->prev;
      blk_hdr *t_blk = (blk_hdr *)f;
      
      t_blk->size = n_blk->size + ((*f)&~1);
      size_t *ftr = (size_t *)(((char *)t_blk + (t_blk->size&~1) - SIZE_T_SIZE));
      *ftr = (t_blk->size)&~1;
      return t_blk;
    }
    return NULL;
  }
  else 
  {
    if (((*nxt)&1) == 0 &&  ((*prv)&1) == 0)
    {
      blk_hdr *p_blk = (blk_hdr *)((char *)f - *prv);
      blk_hdr *n_blk = (blk_hdr *)((char *)f + ((*f)&~1));

      p_blk->prev->next = p_blk->next;
      p_blk->next->prev = p_blk->prev;
      n_blk->prev->next = n_blk->next;
      n_blk->next->prev = n_blk->prev;

      p_blk->size = p_blk->size + ((*f)&~1) + n_blk->size;
      size_t *ftr = (size_t *)(((char *)p_blk + p_blk->size) - SIZE_T_SIZE);
      *ftr = (p_blk->size)&~1;
      return p_blk;     
    }
    else if (((*nxt)&1) == 1 && ((*prv)&1) == 0) 
    {
      blk_hdr *p_blk = (blk_hdr *)((char *)f - *prv);
      p_blk->prev->next = p_blk->next;
      p_blk->next->prev = p_blk->prev;
      p_blk->size = p_blk->size + ((*f)&~1);
      
      size_t *ftr = (size_t *)(((char *)p_blk + p_blk->size) - SIZE_T_SIZE);
      *ftr = (p_blk->size) & ~1;
      return p_blk;     
    }
    else if (((*nxt)&1) == 0 && ((*prv)&1) == 1)
    {
      blk_hdr *n_blk = (blk_hdr *)((char *)f + ((*f)&~1));
      n_blk->prev->next = n_blk->next;
      n_blk->next->prev = n_blk->prev;
      
      blk_hdr * t_blk = (blk_hdr *) f;
      t_blk->size = n_blk->size + ((*f)&~1);
      
      size_t *ftr = (size_t *)(((char *)t_blk + t_blk->size) - SIZE_T_SIZE);
      *ftr = (t_blk->size) & ~1;
      return t_blk;     
    }
    else
      return NULL;
  }
}

