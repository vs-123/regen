#include "regen.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

int intrprt_bars (const char *, size_t, const char *, regen_t *, size_t);

size_t
get_subexpr_len (const char *regex, size_t len)
{
   size_t size = 0;
   size_t i    = 0;

   while (i < len)
      {
         if (regex[i] == '\\')
            {
               if (regex[i + 1] != 'b')
                  {
                     size++;
                  }
               i += 2;
            }
         else if (regex[i] == '^' || regex[i] == '$')
            {
               i++;
            }
         else if (regex[i] == '[')
            {
               size++;
               while (i < len && regex[i] != ']')
                  {
                     i++;
                  }
               i++;
            }
         else
            {
               size++;
               i++;
            }
      }

   return size;
} /* get_subexpr_len */

void
clear_captures (regen_t *regen, size_t p_idx)
{
   if (p_idx > 0 && p_idx <= regen->captures.count)
      {
         regen->captures.elems[p_idx - 1].ptr = NULL;
         regen->captures.elems[p_idx - 1].size = 0;
      }
}

int
match_sngl (const char *regex, const char *str, regen_t *regen)
{
   if (*regex == '.')
      {
         return 1;
      }

   if (*regex == '[')
      {
         int has_found      = 0;
         int is_carrot_mode = (regex[1] == '^');      /* carrot's funnier than caret lol */
         size_t idx         = is_carrot_mode ? 2 : 1; /* 2 or 1 cuz we wanna skip `[^`/`[` */

         while (regex[idx] && regex[idx] != ']')
            {
               if (regex[idx + 1] == '-' && regex[idx + 2] && regex[idx + 2] != ']')
                  {
                     if (*str >= regex[idx] && *str <= regex[idx + 2])
                        {
                           has_found = 1;
                        }
                     idx += 3;
                  }
               else
                  {
                     if (regex[idx] == *str)
                        {
                           has_found = 1;
                        }
                     idx++;
                  }
            }
         if (is_carrot_mode)
            {
               has_found = !has_found;
            }
         return has_found ? 1 : -1;
      }

   if (*regex == '\\')
      {
         switch (regex[1])
            {
            case 'd':
               return (isdigit (*str)) ? 1 : -1;
            case 's':
               return (isspace (*str)) ? 1 : -1;
            case 'w':
               return (isalnum (*str) || *str == '_') ? 1 : -1;
            case 'D':
               return (!isdigit (*str)) ? 1 : -1;
            case 'S':
               return (!isspace (*str)) ? 1 : -1;
            default:
               return (regex[1] == *str) ? 1 : -1;
            }
      }

   char rc = (regen->is_not_case_sensitive) ? tolower (*regex) : *regex;
   char sc = (regen->is_not_case_sensitive) ? tolower (*str) : *str;
   return (rc == sc) ? 1 : -1;
} /* match_sngl */

/* i have no idea how to name this lol */
size_t
get_thing_size (const char *regex, regen_t *regen)
{
   if (regex[0] == '\\')
      {
         return 2;
      }
   if (regex[0] == '[')
      {
         size_t k = 1;
         if (regex[k] == '^')
            {
               k++;
            }
         if (regex[k] == ']')
            {
               k++;
            }
         while (regex[k] && regex[k] != ']')
            {
               k++;
            }
         return k + 1;
      }
   if (regex[0] == '(')
      {
         for (size_t i = 0; i < regen->pairs.count; i++)
            {
               const char *p_start = regen->pairs.elems[i].start;
               if (p_start == regex + 1 || p_start == regex + 3 || p_start == regex + 4)
                  {
                     return (p_start - regex) + regen->pairs.elems[i].size + 1;
                  }
            }
      }
   return 1;
}

int
exec_match (const char *regex, size_t regex_len, const char *str, size_t str_len,
            const char *orig_str, regen_t *regen, size_t p_idx)
{
   size_t i = 0, j = 0;

   while (i < regex_len)
      {
         if (regex[i] == '(')
            {
               size_t target_idx = 0;
               for (size_t pair_idx = 1; pair_idx < regen->pairs.count; pair_idx++)
                  {
                     const char *p_start = regen->pairs.elems[pair_idx].start;
                     if (p_start == (regex + i + 1) || p_start == (regex + i + 3)
                         || p_start == (regex + i + 4))
                        {
                           target_idx = pair_idx;
                           break;
                        }
                  }

               group_type_t type = regen->pairs.elems[target_idx].type;
               int bar_thing;

               if (type == GROUP_CAPTURE) {
                  clear_captures(regen, target_idx);
               }

               if (type == GROUP_LOOKBEHIND_POS || type == GROUP_LOOKBEHIND_NEG)
                  {
                     size_t lb_len = get_subexpr_len (regen->pairs.elems[target_idx].start,
                                                      regen->pairs.elems[target_idx].size);

                     if ((str + j) - lb_len < orig_str)
                        {
                           if (type == GROUP_LOOKBEHIND_POS)
                              {
                                 return -1;
                              }
                           bar_thing = -1;
                        }
                     else
                        {
                           bar_thing = intrprt_bars ((str + j) - lb_len, lb_len, orig_str, regen,
                                                     target_idx);
                        }
                  }
               else
                  {
                     bar_thing = intrprt_bars (str + j, str_len - j, orig_str, regen, target_idx);
                  }

               if (type == GROUP_LOOKAHEAD_POS && bar_thing < 0)
                  {
                     return -1;
                  }
               if (type == GROUP_LOOKAHEAD_NEG && bar_thing >= 0)
                  {
                     return -1;
                  }
               if (type == GROUP_LOOKBEHIND_POS && bar_thing < 0)
                  {
                     return -1;
                  }
               if (type == GROUP_LOOKBEHIND_NEG && bar_thing >= 0)
                  {
                     return -1;
                  }

               if (type == GROUP_CAPTURE || type == GROUP_NON_CAPTURE)
                  {
                     if (bar_thing < 0)
                        {
                           return -1;
                        }

                     if (type == GROUP_CAPTURE && target_idx - 1 < regen->captures.count)
                        {
                           regen->captures.elems[target_idx - 1].ptr  = str + j;
                           regen->captures.elems[target_idx - 1].size = bar_thing;
                        }
                     j += bar_thing;
                  }

               size_t prefix_len = 1;
               if (type == GROUP_LOOKAHEAD_POS || type == GROUP_LOOKAHEAD_NEG)
                  {
                     prefix_len = 3;
                  }
               if (type == GROUP_LOOKBEHIND_POS || type == GROUP_LOOKBEHIND_NEG)
                  {
                     prefix_len = 4;
                  }

               i += prefix_len + regen->pairs.elems[target_idx].size + 1;
               continue;

               continue;
            }

         if (regex[i] == '^')
            {
               if (str + j != orig_str)
                  {
                     return -1;
                  }
               i++;
               continue;
            }

         if (regex[i] == '$')
            {
               return (j == str_len) ? j : -1;
            }

         if (regex[i] == '\\' && regex[i + 1] == 'b')
            {
               int prev_is_word = 0;
               int crnt_is_word = 0;

               if (str + j > orig_str)
                  {
                     char prev    = *(str + j - 1);
                     prev_is_word = (isalnum (prev) || prev == '_');
                  }

               if (j < str_len)
                  {
                     char curr    = *(str + j);
                     crnt_is_word = (isalnum (curr) || curr == '_');
                  }

               if (prev_is_word == crnt_is_word)
                  {
                     return -1;
                  }

               i += 2;
               continue;
            }

         size_t step = get_thing_size (regex + i, regen);

         if (i + step < regex_len
             && (regex[i + step] == '*' || regex[i + step] == '+' || regex[i + step] == '?'))
            {
               char step_c           = regex[i + step];
               int is_lazy           = (i + step + 1 < regex_len && regex[i + step + 1] == '?');
               size_t next_regex_off = i + step + 1 + (is_lazy ? 1 : 0);

               size_t j_strt      = j;
               size_t max_matches = (step_c == '?') ? 1 : (str_len - j);
               size_t min_matches = (step_c == '+') ? 1 : 0;

               int is_group            = (regex[i] == '(');
               size_t target_group_idx = 0;
               if (is_group)
                  {
                     for (size_t pair_idx = 1; pair_idx < regen->pairs.count; pair_idx++)
                        {
                           const char *p_start = regen->pairs.elems[pair_idx].start;
                           if (p_start == (regex + i + 1) || p_start == (regex + i + 3)
                               || p_start == (regex + i + 4))
                              {
                                 target_group_idx = pair_idx;
                                 break;
                              }
                        }
                  }

               if (is_lazy)
                  {
                     /* NON-GREEDY */
                     size_t crnt_offset = 0;
                     for (size_t count = 0; count <= max_matches; count++)
                        {
                           if (count >= min_matches)
                              {
                                 int res = exec_match (
                                     regex + next_regex_off, regex_len - next_regex_off,
                                     str + j_strt + crnt_offset, str_len - (j_strt + crnt_offset),
                                     orig_str, regen, p_idx);
                                 if (res >= 0)
                                    {
                                       return (j_strt + crnt_offset + res);
                                    }
                              }

                           int consumed
                               = is_group
                                     ? intrprt_bars (str + j_strt + crnt_offset,
                                                     str_len - (j_strt + crnt_offset), orig_str,
                                                     regen, target_group_idx)
                                     : match_sngl (regex + i, str + j_strt + crnt_offset, regen);

                           if (consumed <= 0 || (j_strt + crnt_offset + consumed) > str_len)
                              {
                                 break;
                              }
                           crnt_offset += consumed;
                        }
                  }
               else
                  {
                     /* GREEDY */
                     size_t offsets[1024];
                     size_t crnt_total = 0;
                     size_t count      = 0;
                     offsets[0]        = 0;

                     while (count < max_matches)
                        {
                           int consumed
                               = is_group
                                     ? intrprt_bars (str + j_strt + crnt_total,
                                                     str_len - (j_strt + crnt_total), orig_str,
                                                     regen, target_group_idx)
                                     : match_sngl (regex + i, str + j_strt + crnt_total, regen);

                           if (consumed <= 0 || (j_strt + crnt_total + consumed) > str_len)
                              {
                                 break;
                              }
                           crnt_total += consumed;
                           count++;
                           offsets[count] = crnt_total;
                        }

                     while (1)
                        {
                           int res = exec_match (regex + next_regex_off, regex_len - next_regex_off,
                                                 str + j_strt + offsets[count],
                                                 str_len - (j_strt + offsets[count]), orig_str,
                                                 regen, p_idx);
                           if (res >= 0)
                              {
                                 return (j_strt + offsets[count] + res);
                              }

                           if (count == 0)
                              {
                                 break;
                              }

                           count--;
                        }
                  }
               return -1;
            }
         else
            {
               if (j >= str_len || match_sngl (regex + i, str + j, regen) < 0)
                  {
                     return -1;
                  }
               j++;
               i += step;
            }
      }
   return j;
} /* exec_match */

int
intrprt_bars (const char *str, size_t str_len, const char *orig_str, regen_t *regen, size_t p_idx)
{
   paren_pair_t *pair = &regen->pairs.elems[p_idx];
   for (size_t i = 0; i <= pair->bars_count; ++i)
      {
         /* const char *bstart = (i == 0) ? pair->start : regen->bars.elems[pair->bars_idx + i -
          * 1].bar_ptr + 1; */
         const char *bstart
             = (i == 0) ? pair->start : regen->bars.elems[pair->bars_idx + i - 1].bar_ptr + 1;
         size_t b_len;

         if (i < pair->bars_count)
            {
               b_len = regen->bars.elems[pair->bars_idx + i].bar_ptr - bstart;
            }
         else
            {
               b_len = pair->start + pair->size - bstart;
            }

         int result = exec_match (bstart, b_len, str, str_len, orig_str, regen, p_idx);
         if (result >= 0)
            {
               return result;
            }
      }
   return -1;
}

regen_result_t
regen_match (const char *regex, const char *str, regen_t *regen)
{
   regen_result_t result = { 0 };
   size_t regex_len      = strlen (regex);
   regen->bars.count     = 0;
   regen->captures.count = 0;
   regen->pairs.count    = 0;

   paren_pair_t root = { regex, regex_len, 0, 0, GROUP_CAPTURE };
   dappend (regen->pairs, root);

   size_t stack[256];
   size_t stack_ptr   = 0;
   stack[stack_ptr++] = 0;

   for (size_t i = 0; i < regex_len; i++)
      {
         if (regex[i] == '(')
            {
               group_type_t type = GROUP_CAPTURE;
               size_t offset     = 1;

               /* check for lookahead -- (?=, (?! */
               if ((i + 2) < regex_len && regex[i + 1] == '?')
                  {
                     if (regex[i + 2] == ':')
                        {
                           type   = GROUP_NON_CAPTURE;
                           offset = 3;
                        }
                     else if (regex[i + 2] == '=')
                        {
                           type   = GROUP_LOOKAHEAD_POS;
                           offset = 3;
                        }
                     else if (regex[i + 2] == '!')
                        {
                           type   = GROUP_LOOKAHEAD_NEG;
                           offset = 3;
                        }
                     else if (i + 3 < regex_len && regex[i + 2] == '<')
                        {
                           if (regex[i + 3] == '=')
                              {
                                 type = GROUP_LOOKBEHIND_POS;
                              }
                           else if (regex[i + 3] == '!')
                              {
                                 type = GROUP_LOOKBEHIND_NEG;
                              }
                           offset = 4;
                        }
                  }

               paren_pair_t p = { regex + i + offset, 0, regen->bars.count, 0, type };
               capture_t c    = { 0 };

               dappend (regen->pairs, p);
               dappend (regen->captures, c);
               stack[stack_ptr++] = regen->pairs.count - 1;

               i += (offset - 1);
            }
         else if (regex[i] == ')' && stack_ptr > 1)
            {
               size_t p_idx                   = stack[--stack_ptr];
               regen->pairs.elems[p_idx].size = regex + i - regen->pairs.elems[p_idx].start;
            }
         else if (regex[i] == '|')
            {
               size_t crnt_p = stack[stack_ptr - 1];
               bar_t b       = { crnt_p, regex + i };
               dappend (regen->bars, b);
               regen->pairs.elems[crnt_p].bars_count++;
            }
      }

   size_t str_len = strlen (str);
   for (size_t i = 0; i <= str_len; i++)
      {
         int match_len = intrprt_bars (str + i, str_len - i, str, regen, 0);
         if (match_len >= 0)
            {
               char *full_match = malloc (match_len + 1);
               if (full_match)
                  {
                     memcpy (full_match, str + i, match_len);
                     full_match[match_len] = '\0';
                     dappend (result, full_match);
                  }

               for (size_t c = 0; c < regen->captures.count; c++)
                  {
                     capture_t *cap = &regen->captures.elems[c];
                     if (cap->ptr)
                        {
                           char *sub = malloc (cap->size + 1);
                           if (sub)
                              {
                                 memcpy (sub, cap->ptr, cap->size);
                                 sub[cap->size] = '\0';
                                 dappend (result, sub);
                              }
                        }
                     else
                        {
                           /*dappend (result, NULL);*/
                        }
                  }
               return result;
            }
      }
   return result;
} /* regen_match */

void
regen_free (regen_t *r)
{
   if (r->pairs.elems)
      {
         free (r->pairs.elems);
      }
   if (r->bars.elems)
      {
         free (r->bars.elems);
      }
   if (r->captures.elems)
      {
         free (r->captures.elems);
      }
   memset (r, 0, sizeof (regen_t));
}

void
regen_result_free (regen_result_t *r)
{
   for (size_t i = 0; i < r->count; i++)
      {
         free (r->elems[i]);
      }
   free (r->elems);
   r->elems = NULL;
   r->count = 0;
}
