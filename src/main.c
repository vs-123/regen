#include <stdio.h>

#include "regen.h"

int
main (void)
{
   regen_t regen          = { 0 };
   const char *demos[][2] = {
      /* MATCH */
      { "(red|blue) pill", "take the red pill" },
      { "[0-9]+ degrees", "reclined at an angle of 30 degrees" },
      { "\\w+@\\w+\\.\\w+", "mail: vs-123@github.com" },
      { "colou?r", "the colour is grey" },
      { "colou?r", "the color is grey" },
      { "^start", "start of the line" },
      { "end$", "this is the end" },

      /* NO MATCH */
      { "(red|blue) pill", "take the green pill" },
      { "[0-9]+ degrees", "reclined at an angle of 3O degrees" },
      { "\\w+@\\w+\\.\\w+", "mail: vs-123@@github.com" },
      { "colou?r", "the colr is grey" },
      { "^start", "sssssstart of the line" },
      { "end$", "this is the endd" },

      /* LOOKAROUNDS */
      { "abc(?=def)", "abcdef" },
      { "abc(?!def)", "abcghi" },
      { "(?<=abc)def", "abcghi" },
      { "(?<!abc)def", "ghidef" },

      { NULL, NULL },
   };

   for (int i = 0; demos[i][0] != NULL; i++)
      {
         int size = regen_match (demos[i][0], demos[i][1], &regen);
         printf ("[PATTERN #%d] %-17s -->   %s\n", i + 1, demos[i][0],
                 (size >= 0) ? "MATCH" : "NO MATCH");
         regen_free (&regen);
      }

   return 0;
}
