# REGEN

Regen is a dead-simple, **PCRE-compatible**, regular-expressions engine written in C99. The term `regen` is a syllabic abbreviation of **reg**ex-**en**gine.

## USAGE

Regen follows a three-step process -- initialise, match, clean.

The following snippet demonstrates the process along with general usage:

```c
#include "regen.h"

regen_t engine = {0};

/* BASIC MATCHING */
const char *pattern = "colou?r";
const char *text = "the colour is blue";

int match_len = regen_match(pattern, text, &engine);

if (match_len >= 0) {
   printf("MATCH SIZE --> %d characters\n", match_len);
}

/* CAPTURE GROUPS */
const char *pattern = "([A-Za-z]+): (\\d+)";
const char *text = "ContentLength: 404";

if (regen_match(pattern, text, &engine) >= 0) {
   for (size_t i = 0; i < engine.captures.count; i++) {
      capture_t c = engine.captures.elems[i];
      printf("GROUP [%zu]: %.*s\n", i + 1, (int)c.size, c.ptr);
   }
}

/* LOOKAROUNDS (PCRE-COMPATIBLE) */
regen_match("abc(?=def)", "abcdef", &engine); 
regen_match("(?<=abc)def", "abcdef", &engine);

/* TOGGLE CASE-SENSITIVITY */
engine.is_not_case_sensitive = 1;
regen_match("APPLE", "i like apples", &engine); /* matches */

regen_free(&engine);
```

## FEATURES

- **ZERO-WIDTH LOOKAROUND ASSERTIONS**
   - Positive Lookahead -- `(?=...)`
   - Negative Lookahead -- `(?!...)`
   - Positive Lookbehind -- `(?<=...)`
   - Negative Lookbehind -- `(?<!...)`

- **CAPTURING GROUPS + NESTING**
   - Allows deeply nested groups
   - Supports dynamic group discovery

- **ADVANCED ALTERNATION**
   - Tries branches left to right
   - Works inside groups and at root level
   - Supports backtracking i.e. if a branch matches but the rest of the regex fails, regen backtracks and tries the next alternative

- **GREEDY QUANTIFIERS + BACKTRACKING**
   - First consumes as many characters as possible
   - If subsequent part of regex fails due to this, regen backtracks and tries rest of the pattern
   - This makes `[0-9]+[0-9]` work

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
| `*` | **ZERO OR MORE** |  Matches zero or more occurrences |
| `+` | **ONE OR MORE** |  Matches one or more occurences |
| `?` | **ZERO OR ONE** |  Matches zero or one occurence, good for optional components |
| `\|` | **OR OPERATOR** |  Match first pattern or the other, chainable |
| `()` | **CAPTURE GROUPS** |  Group & extract sub-matches (can be nested) |
| `.` | **WILDCARDS** |  Matches any single character |
| `[abc]` | **SETS** |  Match any character contained within the set |
| `[a-z]` | **RANGES** |  Set with ASCII ranges for letters and numbers |
| `[^abc]` | **INVERSION** |  Matches any character NOT in the set |
| `(?=...)` | **LOOKAHEAD** |  Match only if the pattern follows |
| `(?!...)` | **NEG. LOOKAHEAD** |  Match only if the pattern DOES NOT follow |
| `(?<=...)` | **LOOKBEHIND** |  Match only if the pattern precedes |
| `(?<!...)` | **NEG. LOOKBEHIND** |  Match only if the pattern DOES NOT precede |
| `\d` / `\D` | - |  Digits/Non-digits |
| `\s` / `\S` | - |  Whitespace/Non-whitespace |
| `\w` | - |  Alphanumeric words (underscores are counted) |
| `\\` | - |  Literal escape |

## LIMITATIONS

This project does **NOT** support:
- **NON-GREEDY QUANTIFIERS** -- `*?`, `+?`
- **BACKREFERENCES** -- `\1`, `\2`
- **VARIABLE-LENGTH LOOKBEHIND** -- `(?<=a+)`

## LICENSE

This project is released under the GNU Affero General Public License version 3.0 or later.

**NO WARRANTY PROVIDED**

For full terms, see the `LICENSE` file in the project root or visit **https://www.gnu.org/licenses/agpl-3.0.en.html**.
