#include "regen.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

int intrprt_bars(const char *, size_t, regen_t *, size_t);

int match_sngl(const char *regex, const char *str)
{
   if (*regex == '.') {
      return 1;
   }

   if (*regex == '[') {
      int has_found      = 0;
      int is_carrot_mode = (regex[1] == '^'); /* carrot's funnier than caret lol */
      size_t idx         = is_carrot_mode ? 2 : 1; /* 2 or 1 cuz we wanna skip `[^`/`[` */

      while (regex[idx] && regex[idx] != ']') {
         if (regex[idx + 1] == '-' && regex[idx + 2] && regex[idx + 2] != ']') {
            if (*str >= regex[idx] && *str <= regex[idx + 2]) {
               has_found = 1;
            }

            idx += 3;
         } else {
            if (regex[idx] == *str) {
               has_found = 1;
            }

            idx++;
         }
      }

      if (is_carrot_mode) {
         has_found = !has_found;
      }

      return has_found ? 1 : -1;
   }

   if (*regex == '\\') {
      switch (regex[1]) {
      case 'd':
         return isdigit(*str) ? 1 : -1;
      case 's':
         return isspace(*str) ? 1 : -1;
      case 'w':
         return (isalnum(*str) || *str == '_') ? 1 : -1;
      default:
         return regex[1] == *str;
      }
   }

   return *regex == *str;
}

/* i have no idea how to name this lol */
size_t get_thing_size(const char *regex)
{
   if (regex[0] == '\\') {
      return 2;
   }

   if (regex[0] == '[') {
      size_t i = 1;
      while (regex[i] && regex[i] != ']') {
         i++;
      }
      return i + 1;
   }

   return 1;
}

int exec_match(const char *regex, size_t regex_len, const char *str, size_t str_len, regen_t *regen, size_t p_idx)
{
   size_t i = 0, j = 0;

   while (i < regex_len) {
      if (regex[i] == '(') {
         size_t target_idx = 0;

         for (size_t k = 1; k < regen->pairs.count; k++) {
            if (regen->pairs.elems[k].start == (regex + i + 1)) {
               target_idx = k;
               break;
            }
         }

         int ok = intrprt_bars(str + j, str_len - j, regen, target_idx);
         if (ok < 0) {
            return -1;
         }

         if (target_idx - 1 < regen->captures.count) {
            regen->captures.elems[target_idx - 1].ptr  = str + j;
            regen->captures.elems[target_idx - 1].size = ok;
         }

         j += ok;
         i += regen->pairs.elems[target_idx].size + 2;
         continue;
      }

      if (regex[i] == '^') {
         if (j != 0) {
            return -1;
         }
         i++;
         continue;
      }

      if (regex[i] == '$') {
         return (j == str_len) ? j : -1;
      }

      size_t step = get_thing_size(regex + i);

      if (i + step < regex_len && (regex[i + step] == '*' || regex[i + step] == '+')) {
         char step_c   = regex[i + step];
         size_t j_strt = j;

         while (j < str_len && match_sngl(regex + i, str + j) > 0) {
            j++;
         }

         size_t min_j = (step_c == '+') ? j_strt + 1 : j_strt;

         while (j >= min_j) {
            /* this took wayy too long for me to adjust */
            int res = exec_match(regex + i + step + 1, regex_len - (i + step + 1),
                str + j, str_len - j, regen, p_idx);

            if (res >= 0) {
               return j + res;
            }

            if (j == 0) {
               break;
            }

            j--;
         }

         return -1;
      } else {
         if (j >= str_len || match_sngl(regex + i, str + j) < 0) {
            return -1;
         }

         j++;
         i += step;
      }
   }
   return j;
}

int intrprt_bars(const char *str, size_t str_len, regen_t *regen, size_t p_idx)
{
   paren_pair_t *pair = &regen->pairs.elems[p_idx];

   for (size_t i = 0; i <= pair->bars_count; ++i) {
      const char *bstart;
      size_t b_len;

      if (i == 0) {
         bstart = pair->start;
      } else {
         bstart = regen->bars.elems[pair->bars_idx + i - 1].bar_ptr + 1;
      }

      if (i < pair->bars_count) {
         b_len = regen->bars.elems[pair->bars_idx + i].bar_ptr - bstart;
      } else {
         b_len = pair->start + pair->size - bstart;
      }

      int result = exec_match(bstart, b_len, str, str_len, regen, p_idx);

      if (result >= 0) {
         return result;
      }
   }

   return -1;
}

int regen_match(const char *regex, const char *str, regen_t *regen)
{
   size_t regex_len      = strlen(regex);
   regen->bars.count     = 0;
   regen->captures.count = 0;
   regen->pairs.count    = 0;

   paren_pair_t root = { regex, regex_len, 0, 0 };
   dappend(regen->pairs, root);

   size_t stack[256];
   size_t stack_ptr   = 0;
   stack[stack_ptr++] = 0;

   for (size_t i = 0; i < regex_len; i++) {
      if (regex[i] == '(') {
         paren_pair_t p = { regex + i + 1, 0, regen->bars.count, 0 };
         capture_t c    = { 0 };

         dappend(regen->pairs, p);
         dappend(regen->captures, c);

         stack[stack_ptr] = regen->pairs.count - 1;
         stack_ptr++;
      } else if (regex[i] == ')' && stack_ptr > 1) {
         size_t p_idx                   = stack[--stack_ptr];
         regen->pairs.elems[p_idx].size = (size_t)(regex + i - regen->pairs.elems[p_idx].start);
      } else if (regex[i] == '|') {
         size_t crnt_p = stack[stack_ptr - 1];
         bar_t b       = { crnt_p, regex + i };
         dappend(regen->bars, b);
         regen->pairs.elems[crnt_p].bars_count++;
      }
   }

   size_t str_len = strlen(str);
   for (size_t i = 0; i <= str_len; i++) {
      int ok = intrprt_bars(str + i, str_len - i, regen, 0);
      if (ok >= 0) {
         return ok;
      }
   }

   return -1;
}

void regen_free(regen_t *r)
{
   free(r->pairs.elems);
   free(r->bars.elems);
   free(r->captures.elems);
}
