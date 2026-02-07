#include "regen.h"

#include <stdint.h>

#define dappend(vector, i)                                                                   \
   do {                                                                                      \
      if (vector.count >= vector.capacity) {                                                 \
         vector.capacity = (vector.capacity) ? vector.capacity * 2 : 256;                    \
         vector.elems    = realloc(vector.elems, vector.capacity * sizeof(vector.elems[0])); \
      }                                                                                      \
      vector.elems[vector.count++] = i;                                                      \
   } while (0);

typedef struct {

} paren_pair_t;

typedef struct {
   paren_pair_t* elems;
   size_t count;
   size_t capacity;
} dparen_pair_t;

typedef struct {
} bar_t;

typedef struct {
   bar_t* elems;
   size_t count;
   size_t capacity;
} dbar_t;

typedef struct {
} capture_t;

typedef struct {
   capture_t* elems;
   size_t count;
   size_t capacity;
} dcapture_t;

typedef struct regen_t {
   dparen_pair_t pairs;
   dbar_t bars;
   dcapture_t captures;
} regen_t;

int regen_exec(const char* str, const char* regex)
{
}
