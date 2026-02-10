#ifndef REGEN_H
#define REGEN_H

#include <stddef.h>
#include <stdint.h>

#define dappend(vector, i)                                                                         \
   do                                                                                              \
      {                                                                                            \
         if ((vector).count >= (vector).capacity)                                                  \
            {                                                                                      \
               (vector).capacity = ((vector).capacity) ? (vector).capacity * 2 : 256;              \
               void *ptr                                                                           \
                   = realloc ((vector).elems, (vector).capacity * sizeof ((vector).elems[0]));     \
               if (ptr)                                                                            \
                  (vector).elems = ptr;                                                            \
            }                                                                                      \
         (vector).elems[(vector).count++] = i;                                                     \
      }                                                                                            \
   while (0);

typedef enum
{
   GROUP_CAPTURE,
   GROUP_NON_CAPTURE,
   GROUP_LOOKAHEAD_POS,
   GROUP_LOOKAHEAD_NEG,
   GROUP_LOOKBEHIND_POS,
   GROUP_LOOKBEHIND_NEG,
} group_type_t;

typedef struct
{
   const char *start;
   size_t size;
   size_t bars_idx;
   size_t bars_count;
   group_type_t type;
} paren_pair_t;

typedef struct
{
   paren_pair_t *elems;
   size_t count;
   size_t capacity;
} dparen_pair_t;

typedef struct
{
   size_t paren_idx; /* which paren pair does this belong to */
   const char *bar_ptr;
} bar_t;

typedef struct
{
   bar_t *elems;
   size_t count;
   size_t capacity;
} dbar_t;

typedef struct
{
   const char *ptr;
   size_t size;
} capture_t;

typedef struct
{
   capture_t *elems;
   size_t count;
   size_t capacity;
} dcapture_t;


typedef enum {
   REGEN_RES_OK,
   REGEN_RES_NOMATCH,
   REGEN_RES_ERROR,
   REGEN_RES_ERROR_INVALID_BACKREF,
   REGEN_RES_ERROR_NESTED_BACKREF,
} regen_result_status_t;

typedef struct
{
   regen_result_status_t status;
   size_t count;
   size_t capacity;
   char **elems;
} regen_result_t;

typedef struct
{
   dparen_pair_t groups;
   dbar_t alternatives;
   dcapture_t captures;
   int is_not_case_sensitive;
} regen_t;

regen_result_t regen_match (const char *regexp, const char *str, regen_t *regen);
void regen_free (regen_t *r);
void regen_result_free (regen_result_t *r);

#endif /* REGEN_H */
