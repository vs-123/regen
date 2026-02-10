#include "regen.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

int match_alternation (const char *, size_t, const char *, regen_t *, size_t);

size_t
calc_fixed_width (const char *regex, size_t len)
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
         regen->captures.elems[p_idx - 1].ptr  = NULL;
         regen->captures.elems[p_idx - 1].size = 0;
      }
}

int
match_atom (const char *regex, const char *str, regen_t *regen)
{
   if (*regex == '.')
      {
         return 1;
      }

   if (*regex == '[')
      {
         int has_found  = 0;
         int is_negated = (regex[1] == '^');
         size_t idx     = is_negated ? 2 : 1; /* 2 or 1 cuz we wanna skip `[^`/`[` */

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
         if (is_negated)
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

size_t
get_token_len (const char *regex, regen_t *regen)
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
         for (size_t i = 0; i < regen->groups.count; i++)
            {
               const char *p_start = regen->groups.elems[i].start;
               if (p_start == regex + 1 || p_start == regex + 3 || p_start == regex + 4)
                  {
                     return (p_start - regex) + regen->groups.elems[i].size + 1;
                  }
            }
      }
   return 1;
}

int
match_recursive (const char *regex, size_t regex_len, const char *str, size_t str_len,
                 const char *orig_str, regen_t *regen, size_t p_idx)
{
   size_t i = 0, j = 0;

   while (i < regex_len)
      {
         if (regex[i] == '(')
            {
               size_t target_idx = 0;
               for (size_t pair_idx = 1; pair_idx < regen->groups.count; pair_idx++)
                  {
                     const char *p_start = regen->groups.elems[pair_idx].start;
                     if (p_start == (regex + i + 1) || p_start == (regex + i + 3)
                         || p_start == (regex + i + 4))
                        {
                           target_idx = pair_idx;
                           break;
                        }
                  }

               group_type_t type = regen->groups.elems[target_idx].type;
               int consumed;

               if (type == GROUP_CAPTURE)
                  {
                     clear_captures (regen, target_idx);
                  }

               if (type == GROUP_LOOKBEHIND_POS || type == GROUP_LOOKBEHIND_NEG)
                  {
                     size_t lb_len = calc_fixed_width (regen->groups.elems[target_idx].start,
                                                       regen->groups.elems[target_idx].size);

                     if ((str + j) - lb_len < orig_str)
                        {
                           if (type == GROUP_LOOKBEHIND_POS)
                              {
                                 return -1;
                              }
                           consumed = -1;
                        }
                     else
                        {
                           consumed = match_alternation ((str + j) - lb_len, lb_len, orig_str,
                                                         regen, target_idx);
                        }
                  }
               else
                  {
                     consumed
                         = match_alternation (str + j, str_len - j, orig_str, regen, target_idx);
                  }

               if (type == GROUP_LOOKAHEAD_POS && consumed < 0)
                  {
                     return -1;
                  }
               if (type == GROUP_LOOKAHEAD_NEG && consumed >= 0)
                  {
                     return -1;
                  }
               if (type == GROUP_LOOKBEHIND_POS && consumed < 0)
                  {
                     return -1;
                  }
               if (type == GROUP_LOOKBEHIND_NEG && consumed >= 0)
                  {
                     return -1;
                  }

               if (type == GROUP_CAPTURE || type == GROUP_NON_CAPTURE)
                  {
                     if (consumed < 0)
                        {
                           return -1;
                        }

                     if (type == GROUP_CAPTURE && target_idx - 1 < regen->captures.count)
                        {
                           regen->captures.elems[target_idx - 1].ptr  = str + j;
                           regen->captures.elems[target_idx - 1].size = consumed;
                        }
                     j += consumed;
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

               i += prefix_len + regen->groups.elems[target_idx].size + 1;
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

         size_t token_len = get_token_len (regex + i, regen);

         if (i + token_len < regex_len
             && (regex[i + token_len] == '*' || regex[i + token_len] == '+'
                 || regex[i + token_len] == '?'))
            {
               char step_c = regex[i + token_len];
               int is_lazy = (i + token_len + 1 < regex_len && regex[i + token_len + 1] == '?');
               size_t next_regex_off = i + token_len + 1 + (is_lazy ? 1 : 0);

               size_t j_offset    = j;
               size_t max_matches = (step_c == '?') ? 1 : (str_len - j);
               size_t min_matches = (step_c == '+') ? 1 : 0;

               int is_group            = (regex[i] == '(');
               size_t target_group_idx = 0;
               if (is_group)
                  {
                     for (size_t pair_idx = 1; pair_idx < regen->groups.count; pair_idx++)
                        {
                           const char *p_start = regen->groups.elems[pair_idx].start;
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
                                 int res = match_recursive (
                                     regex + next_regex_off, regex_len - next_regex_off,
                                     str + j_offset + crnt_offset,
                                     str_len - (j_offset + crnt_offset), orig_str, regen, p_idx);
                                 if (res >= 0)
                                    {
                                       return (j_offset + crnt_offset + res);
                                    }
                              }

                           if (count >= max_matches)
                              {
                                 break;
                              }

                           int consumed
                               = is_group
                                     ? match_alternation (str + j_offset + crnt_offset,
                                                          str_len - (j_offset + crnt_offset),
                                                          orig_str, regen, target_group_idx)
                                     : match_atom (regex + i, str + j_offset + crnt_offset, regen);

                           if (consumed <= 0 || (j_offset + crnt_offset + consumed) > str_len)
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
                                     ? match_alternation (str + j_offset + crnt_total,
                                                          str_len - (j_offset + crnt_total),
                                                          orig_str, regen, target_group_idx)
                                     : match_atom (regex + i, str + j_offset + crnt_total, regen);

                           if (consumed <= 0 || (j_offset + crnt_total + consumed) > str_len)
                              {
                                 break;
                              }
                           crnt_total += consumed;
                           count++;
                           offsets[count] = crnt_total;
                        }

                     while (count >= min_matches)
                        {
                           int res = match_recursive (
                               regex + next_regex_off, regex_len - next_regex_off,
                               str + j_offset + offsets[count],
                               str_len - (j_offset + offsets[count]), orig_str, regen, p_idx);
                           if (res >= 0)
                              {
                                 return (j_offset + offsets[count] + res);
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
               if (j >= str_len || match_atom (regex + i, str + j, regen) < 0)
                  {
                     return -1;
                  }
               j++;
               i += token_len;
            }
      }
   return j;
} /* exec_match */

int
match_alternation (const char *str, size_t str_len, const char *orig_str, regen_t *regen,
                   size_t p_idx)
{
   paren_pair_t *group = &regen->groups.elems[p_idx];
   for (size_t i = 0; i <= group->bars_count; ++i)
      {
         const char *alt_start;

         if (i == 0)
            {
               alt_start = group->start;
            }
         else
            {
               alt_start = regen->alternatives.elems[group->bars_idx + i - 1].bar_ptr + 1;
            }

         size_t alt_len;

         if (i < group->bars_count)
            {
               alt_len = regen->alternatives.elems[group->bars_idx + i].bar_ptr - alt_start;
            }
         else
            {
               alt_len = group->start + group->size - alt_start;
            }

         int result = match_recursive (alt_start, alt_len, str, str_len, orig_str, regen, p_idx);
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
   result.status         = REGEN_RES_NOMATCH;

   size_t regex_len          = strlen (regex);
   regen->alternatives.count = 0;
   regen->captures.count     = 0;
   regen->groups.count       = 0;

   paren_pair_t root = { regex, regex_len, 0, 0, GROUP_CAPTURE };
   dappend (regen->groups, root);

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

               paren_pair_t p = { regex + i + offset, 0, regen->alternatives.count, 0, type };
               capture_t c    = { 0 };

               dappend (regen->groups, p);
               dappend (regen->captures, c);
               stack[stack_ptr++] = regen->groups.count - 1;

               i += (offset - 1);
            }
         else if (regex[i] == ')' && stack_ptr > 1)
            {
               size_t p_idx                    = stack[--stack_ptr];
               regen->groups.elems[p_idx].size = regex + i - regen->groups.elems[p_idx].start;
            }
         else if (regex[i] == '|')
            {
               size_t crnt_p = stack[stack_ptr - 1];
               bar_t b       = { crnt_p, regex + i };
               dappend (regen->alternatives, b);
               regen->groups.elems[crnt_p].bars_count++;
            }
      }

   size_t str_len = strlen (str);
   for (size_t i = 0; i <= str_len; i++)
      {
         int match_len = match_alternation (str + i, str_len - i, str, regen, 0);

         if (match_len < -1)
            {
               result.status = REGEN_RES_ERROR;
               return result;
            }

         if (match_len >= 0)
            {
               result.status = REGEN_RES_OK;

               char *full_match = malloc (match_len + 1);
               if (!full_match)
                  {
                     result.status = REGEN_RES_ERROR;
                     return result;
                  }

               memcpy (full_match, str + i, match_len);
               full_match[match_len] = '\0';
               dappend (result, full_match);

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
                     /*else
                        {
                           dappend (result, NULL);
                        }
                        */
                  }
               return result;
            }
      }

   return result;
} /* regen_match */

void
regen_free (regen_t *r)
{
   if (r->groups.elems)
      {
         free (r->groups.elems);
      }
   if (r->alternatives.elems)
      {
         free (r->alternatives.elems);
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
   if (!r || !r->elems)
      {
         return;
      }

   for (size_t i = 0; i < r->count; i++)
      {
         if (r->elems[i])
            {
               free (r->elems[i]);
            }
      }

   free (r->elems);
   r->elems  = NULL;
   r->count  = 0;
   r->status = REGEN_RES_NOMATCH;
}
