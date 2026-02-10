#include <stdio.h>

#include "regen.h"

typedef struct {
   char* regex;
   char* text;
   int should_match;
} case_t;

#define NULL_CASE (case_t) { .regex = NULL, .text = NULL, .should_match = -1 }

int main(void)
{
   regen_t regen        = { 0 };
   const case_t cases[] = { /* MATCH */
      { "(red|blue) pill", "take the red pill", .should_match = 1 },
      { "[0-9]+ degrees", "reclined at an angle of 30 degrees", .should_match = 1 },
      { "\\w+@\\w+\\.\\w+", "mail: vs-123@github.com", .should_match = 1 },
      { "colou?r", "the colour is grey", .should_match = 1 },
      { "colou?r", "the color is grey", .should_match = 1 },
      { "^start", "start of the line", .should_match = 1 },
      { "end$", "this is the end", .should_match = 1 },

      /* NO MATCH */
      { "(red|blue) pill", "take the green pill", .should_match = 0 },
      { "[0-9]+ degrees", "reclined at an angle of 3O degrees", .should_match = 0 },
      { "\\w+@\\w+\\.\\w+", "mail: vs-123@@github.com", .should_match = 0 },
      { "colou?r", "the colr is grey", .should_match = 0 },
      { "^start", "sssssstart of the line", .should_match = 0 },
      { "end$", "this is the endd", .should_match = 0 },

      /* LOOKAROUNDS */
      { "abc(?=def)", "abcdef", .should_match = 1 }, { "abc(?!def)", "abcghi", .should_match = 1 },
      { "(?<=abc)def", "abcdef", .should_match = 1 },
      { "(?<!abc)def", "ghidef", .should_match = 1 },

      { "(?<=\\bNo )\\d+", "No 101", .should_match = 1 },

      { "<a>.*</a>", "<a>first</a><a>second</a>", .should_match = 1 },
      { "<a>.*?</a>", "<a>first</a><a>second</a>", .should_match = 1 },

      { "([ab])\\1", "aa", .should_match = 1 }, { "([ab])\\1", "ab", .should_match = 0 },
      { "(\\w+)\\s+\\1", "hello hello", .should_match = 1 },
      { "(\\d+) (\\d+) \\1 \\2", "1 2 1 2", .should_match = 1 },
      { "(a|(b))\\1", "a", .should_match = 1 },

      NULL_CASE
   };

   for (int i = 0; cases[i].should_match >= 0; i++) {
      case_t wcase          = cases[i];
      regen_result_t result = regen_match(wcase.regex, wcase.text, &regen);

      printf("=== CASE #%02d ===\n", i + 1);
      printf("   REGEX -- %s\n", wcase.regex);
      printf("   TEXT -- %s\n", wcase.text);

      switch (result.status) {
      case REGEN_RES_NOMATCH: {
         printf("   NO MATCH!!!\n");
      } break;

      case REGEN_RES_OK: {
         printf("   MATCH COUNT -- %02zu\n", result.count);
         printf("   MATCHES -- ");
         for (size_t i = 0; i < result.count; i++) {
            printf("'%s'  ", result.elems[i]);
         }
         printf("\n");
      } break;

      case REGEN_RES_ERROR: {
         printf("   REGEN ERROR!!!\n");
      } break;

      default: {
         printf("   UNEXPECTED RESULT TYPE!!!\n");
      } break;
      }

      printf("\n");

      regen_free(&regen);
      regen_result_free(&result);
   }

   return 0;
}
