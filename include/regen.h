#ifndef REGEN_H
#define REGEN_H

#include <stdint.h>
#include <stddef.h>

#define dappend(vector, i)                                                                    \
   do {                                                                                       \
      if ((vector).count >= (vector).capacity) {                                              \
         (vector).capacity = ((vector).capacity) ? (vector).capacity * 2 : 256;               \
         void *ptr = realloc((vector).elems, (vector).capacity * sizeof((vector).elems[0]));  \
         if (ptr) (vector).elems = ptr;                                                       \
      }                                                                                       \
      (vector).elems[(vector).count++] = i;                                                   \
   } while (0);

typedef struct {
   const char *start;
   size_t size;
   size_t bars_idx;
   size_t bars_count;
} paren_pair_t;

typedef struct {
   paren_pair_t *elems;
   size_t count;
   size_t capacity;
} dparen_pair_t;

typedef struct {
   size_t paren_idx; /* which prren pair does this belong to */
   const char *bar_ptr;
} bar_t;

typedef struct {
   bar_t *elems;
   size_t count;
   size_t capacity;
} dbar_t;

typedef struct {
   const char *ptr;
   size_t size;
} capture_t;

typedef struct {
   capture_t *elems;
   size_t count;
   size_t capacity;
} dcapture_t;

typedef struct {
   dparen_pair_t pairs;
   dbar_t bars;
   dcapture_t captures;
   int is_not_case_sensitive;
} regen_t;

int regen_match(const char *regexp, const char *str, regen_t *regen);
void regen_free(regen_t *r);

#endif /* REGEN_H */
