#include <stdio.h>

#include "regen.h"

typedef struct {
   char *regex;
   char *text;
   int should_match;
} case_t;

#define NULL_CASE (case_t){.regex=NULL,.text=NULL,.should_match=-1}

int
main (void)
{
   regen_t regen          = { 0 };
   /*const char *demos[][2] = {*/
   const case_t cases[] = {
      /* MATCH */
      { "(red|blue) pill", "take the red pill", .should_match=1},
      { "[0-9]+ degrees", "reclined at an angle of 30 degrees", .should_match=1},
      { "\\w+@\\w+\\.\\w+", "mail: vs-123@github.com", .should_match=1},
      { "colou?r", "the colour is grey", .should_match=1},
      { "colou?r", "the color is grey", .should_match=1},
      { "^start", "start of the line", .should_match=1},
      { "end$", "this is the end", .should_match=1},

      /* NO MATCH */
      { "(red|blue) pill", "take the green pill", .should_match=0},
      { "[0-9]+ degrees", "reclined at an angle of 3O degrees", .should_match=0},
      { "\\w+@\\w+\\.\\w+", "mail: vs-123@@github.com", .should_match=0},
      { "colou?r", "the colr is grey", .should_match=0},
      { "^start", "sssssstart of the line", .should_match=0},
      { "end$", "this is the endd", .should_match=0},

      /* LOOKAROUNDS */
      { "abc(?=def)", "abcdef", .should_match=1},
      { "abc(?!def)", "abcghi", .should_match=1},
      { "(?<=abc)def", "abcdef", .should_match=1},
      { "(?<!abc)def", "ghidef", .should_match=1},

      { "(?<=\\bNo )\\d+", "No 101", .should_match=1},

      NULL_CASE
   };

   for (int i = 0; cases[i].should_match >= 0; i++)
      {
         case_t wcase = cases[i];
         int size = regen_match (wcase.regex, wcase.text, &regen);
         int is_match = (size >= 0);
         char *pass_fail = (wcase.should_match == is_match) ? "PASS" : "FAIL";
         printf ("[%s #%02d] /%s/ -- \"%s\"\n", pass_fail, i + 1, wcase.regex, wcase.text);
         regen_free (&regen);
      }

   return 0;
}
