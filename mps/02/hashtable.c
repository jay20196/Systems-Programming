#include <stdlib.h>
#include <string.h>
#include "hashtable.h"

/* Daniel J. Bernstein's "times 33" string hash function, from comp.lang.C;
   See https://groups.google.com/forum/#!topic/comp.lang.c/lSKWXiuNOAk */
unsigned long hash(char *str) {
  unsigned long hash = 5381;
  int c;

  while ((c = *str++))
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return hash;
}

hashtable_t *make_hashtable(unsigned long size) {
  hashtable_t *ht = malloc(sizeof(hashtable_t));
  ht->size = size;
  ht->buckets = calloc(sizeof(bucket_t *), size);
  return ht;
}

void ht_put(hashtable_t *ht, char *key, void *val) {
  /* FIXME: the current implementation doesn't update existing entries */
  unsigned int idx = hash(key) % ht->size;
  
  //If else statement for bucket
  bucket_t *buck = ht->buckets[idx];

  if (buck) {
    while (buck->next && strcmp(buck->key, key) != 0) 
    {
      buck = buck->next;
    }
    
    if (!strcmp(buck->key,key)) 
    {
      free(buck->val); 
      free(key); 
      buck->val = val;
      return;
    } 
    else 
    {  
      bucket_t *b = malloc(sizeof(bucket_t));
      b->key = key;
      b->val = val;
      b->next = ht->buckets[idx];
      ht->buckets[idx] = b;
    }
  } else 
  {
    bucket_t *b = malloc(sizeof(bucket_t));
    b->key = key;
    b->val = val;
    b->next = ht->buckets[idx];
    ht->buckets[idx] = b;
  }
}

void *ht_get(hashtable_t *ht, char *key)
 {
  unsigned int idx = hash(key) % ht->size;
  bucket_t *b = ht->buckets[idx];
  while (b) {
    if (strcmp(b->key, key) == 0)
     {
      return b->val;
    }
    b = b->next;
  }
  return NULL;
}

void ht_iter(hashtable_t *ht, int (*f)(char *, void *)) 
{
  bucket_t *b;
  unsigned long i;
  for (i=0; i<ht->size; i++)
   {
    b = ht->buckets[i];
    while (b) 
    {
      if (!f(b->key, b->val)) 
      {
        return ; // abort iteration
      }
      b = b->next;
    }
  }
}

void free_hashtable(hashtable_t *ht) {
  bucket_t *b, *temp;
  unsigned long i;
  for (i = 0; i < ht->size; i++)
  {
    b = ht->buckets[i];
    while(b)
    {
      temp = b;
      b = b->next;
      free(temp->key);
      free(temp->val);
      free(temp);
    }
  }
  free(ht->buckets);
  free(ht);
   // FIXME: must free all substructures!
}

/* TODO */
void  ht_del(hashtable_t *ht, char *key) {
  unsigned int idx = hash(key) % ht->size;
  bucket_t *b = ht->buckets[idx];
  bucket_t *temp;
  
  if (b) {  
    if(strcmp(b->key, key)==0)
    { 
      temp = b->next;
      ht->buckets[idx] = b->next;
      free(b->val);
      free(b->key);
      free(b);
      ht->buckets[idx] = temp; 
    } else if (b->next) 
    {     
      while(b->next)
      {
        if (strcmp((b->next)->key, key)==0)
        { 
          temp = b->next;
          b->next = temp->next;
          free(temp->key); 
          free(temp->val); 
          free(temp); 
          return;
        } 

        b = b->next;
      }
    }
  }
}

void  ht_rehash(hashtable_t *ht, unsigned long newsize) {
  bucket_t **nbucket = calloc(newsize, sizeof(bucket_t *));
  //ht = make_hashtable(newsize);
  unsigned long i, newidx;
  bucket_t *b, *b_temp;  
  //void *valcpy[sizeof()];
  //char *keycpy;
  for (i = 0; i < ht->size; i++) {
    b = ht->buckets[i];
    while (b) {
      //keycpy = strdup(b->key);
      b_temp = b;
      newidx = hash(b->key) % newsize;
      bucket_t *buck = malloc(sizeof(bucket_t));
      buck->key = b->key;
      buck->val = b->val;
      buck->next = nbucket[newidx];
      nbucket[newidx] = buck;
      b = b->next;
      free(b_temp); 
    } 
  }
  free(ht->buckets);
  ht->size = newsize;
  ht->buckets = nbucket;


  //free(b->key); free(b->val); free(b);
  //free_hashtable(ht); 
  //*ht= *ht_temp;
  //free_hashtable(ht_temp);
  //free(ht_temp);
}
