#include <stdio.h>

#include "regen.h"

int main(void)
{
   regen_t regen = { 0 };
   regen.flags   = 0;

   const char* regex = "([A-Za-z]+): ([0-9]+)";
   const char* txt   = "ContentLength: 404";

   printf("[REGEX] %s\n", regex);
   printf("[TXT] \"%s\"\n\n", txt);

   int match_size = regen_match(regex, txt, &regen);

   if (match_size >= 0) {
      printf("[SUCCESS] MATCH SIZE -- %d\n", match_size);
      printf("\n");
      printf("[CAPTURES]\n");
      for (size_t i = 0; i < regen.captures.count; i++) {
         capture_t cptr = regen.captures.elems[i];

         if (cptr.ptr) {
            printf("[%zu] %.*s\n", i, (int)cptr.size, cptr.ptr);
         }
      }
   } else {
      printf("[FAIL] NO MATCH\n");
   }

   regen_free(&regen);

   return 0;
}
