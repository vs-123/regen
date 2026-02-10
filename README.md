# REGEN

Regen is a dead-simple, **PCRE-compatible**, regular-expressions engine written in C99. The term `regen` is a syllabic abbreviation of **reg**ex-**en**gine. This regular-expressions engine uses a recursive-backtracking algorithm with a transactional state-management system to handle complex pattern matching scenarios.

## USAGE

Regen follows a three-step process -- initialise, match, clean.

The following snippet demonstrates the process along with general usage:

```c
#include <stdio.h>
#include "regen.h"

int main(void) {
   /* INITIALISE */
   regen_t engine = {0};
   regen_result_t result = {0};

   /* BACKREFERENCES + CAPTURE GROUPS */
   const char *pattern = "([ab])\\1"; 
   const char *text = "aa";

   result = regen_match(pattern, text, &engine);

   if (result.status == REGEN_RES_OK) {
      /**
         FULL MATCH    --> result.elems[0]
         FIRST CAPTURE --> result.elems[1]
      */
      printf("MATCH: %s | GROUP 1: %s\n", result.elems[0], result.elems[1]);
   }
   regen_result_free(&result);

   /* CLEANUP */
   regen_free(&engine);
   return 0;
}
```

## FEATURES

- **TRANSACTIONAL BACKTRACKING**
   - Uses a local "undo log" for each recursion depth to ensure integrity of captures
   - Prevents dirty capture states, especially in complex alternations

- **BACKREFERENCES**
   - Supports PCRE-style backreferences like `\1`, `\2`, etc.
   - Safety checks are implemented to avoid infinite recursion in nested backreferences like `(\1)`

- **ZERO-WIDTH LOOKAROUND ASSERTIONS**
   - Positive Lookahead -- `(?=...)`
   - Negative Lookahead -- `(?!...)`
   - Positive Lookbehind -- `(?<=...)`
   - Negative Lookbehind -- `(?<!...)`

- **CAPTURING GROUPS + NESTING**
   - Allows deeply nested groups
   - Supports dynamic group discovery
   - Non-capturing groups `(?:...)` supported too 

- **ADVANCED ALTERNATION**
   - Tries branches left to right
   - Works inside groups and at root level
   - Supports backtracking i.e. if a branch matches but the rest of the regex fails, regen backtracks and tries the next alternative

- **QUANTIFIERS**
   - Greedy quantifiers first consume as many characters as possible
   - If subsequent part of regex fails due to this, regen backtracks and tries rest of the pattern
   - This makes patterns like `[0-9]+[0-9]` work
   - Non-greedy quantifiers like `*?` & `+?` are supported too

- **CHARACTER CLASSES + WILDCARD**
   - Supports `[...]` sets, `[a-z]` ranges, `[^...]` inverted sets, `\d`, `\w`, `\s` and their inversions and also `.` wildcard
   - See **SYNTAX** section for more information

- **ANCHORS + BOUNDARIES**
   - Supports `^` and `$` anchors
   - Supports `\\` escape literals like `\.`
   - Supports `\b` word boundaries

- **CASE SENSITIVITY**
   - Can be toggled with `is_not_case_sensitive` flag

## SYNTAX

| SYNTAX | TERMINOLOGY | DESCRIPTION |
| :-- | :-- | :-- |
| `^` | **START OF STRING** |  Matches only when pattern occurs at the absolute beginning of the input |
| `$` | **END OF STRING** |  Matches only when pattern reaches the very end of the input |
| `.` | **WILDCARDS** |  Matches any single character |
| `*` | **ZERO OR MORE** |  Matches zero or more occurrences |
| `+` | **ONE OR MORE** |  Matches one or more occurences |
| `?` | **ZERO OR ONE** |  Matches zero or one occurence, good for optional components |
| `*?`, `+?` | **NON-GREEDY QUANTIFIERS** |  Matches the minimum required for the quantifier |
| `\|` | **OR OPERATOR** |  Match first pattern or the other, chainable |
| `(...)` | **CAPTURE GROUPS** |  Group & extract sub-matches (can be nested) |
| `(?:...)` | **NON-CAPTURE** |  Groups without saving to capture array |
| `\1`, `\2`, ... | **BACKREFERENCES** |  Matches exact text of a previous group |
| `[abc]` | **SETS** |  Match any character contained within the set |
| `[a-z]` | **RANGES** |  Set with ASCII ranges for letters and numbers |
| `[^abc]` | **INVERSION** |  Matches any character NOT in the set |
| `(?=...)` | **LOOKAHEAD** |  Match only if the pattern follows |
| `(?!...)` | **NEG. LOOKAHEAD** |  Match only if the pattern DOES NOT follow |
| `(?<=...)` | **LOOKBEHIND** |  Match only if the pattern precedes |
| `(?<!...)` | **NEG. LOOKBEHIND** |  Match only if the pattern DOES NOT precede |
| `\d` / `\D` | **SHORTHANDS** |  Digits/Non-digits |
| `\s` / `\S` | **SHORTHANDS** |  Whitespace/Non-whitespace |
| `\w` | **SHORTHAND** |  Alphanumeric words (underscores are counted) |
| `\b` | **BOUNDARY** |  Word boundary |
| `\\` | **ESCAPE** |  Literal escape |

## LIMITATIONS

- **VARIABLE-LENGTH LOOKBEHIND** -- `(?<=a+)` not supported, lookbehinds need fixed-width patterns
- **BACKREFERENCE COUNT** -- Limited to `\1`-`\9`
- **QUANTIFIER LIMITS** -- `{min,max}` not supported

## LICENSE

This project is released under the GNU Affero General Public License version 3.0 or later.

**NO WARRANTY PROVIDED**

For full terms, see the `LICENSE` file in the project root or visit **https://www.gnu.org/licenses/agpl-3.0.en.html**.
