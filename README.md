# REGEN

Regen is a dead-simple regex-engine written in C99. The term `regen` is a syllabic abbreviation of **reg**ex-**en**gine.

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

/* TOGGLE CASE-SENSITIVITY */
engine.is_not_case_sensitive = 1;
regen_match("APPLE", "i like apples", &engine); /* matches */

regen_free(&engine);
```

## FEATURES

- `^` **START OF STRING** -- Matches only when pattern occurs at the absolute beginning of the input
- `$` **END OF STRING** -- Matches only when pattern reaches the very end of the input
- `*` **ZERO OR MORE** -- Matches zero or more occurrences
- `+` **ONE OR MORE** -- Matches one or more occurences
- `?` **ZERO OR ONE** -- Matches zero or one occurence, good for optional components
- `|` **OR OPERATOR** -- Match first pattern or the other, chainable
- `()` **CAPTURE GROUPS** -- Group & extract sub-matches (can be nested)
- `.` **WILDCARDS** -- Matches any single character
- `[abc]` **SETS** -- Match any character contained within the set
- `[a-z]` **RANGES** -- Set with ASCII ranges for letters and numbers
- `[^abc]` **INVERSION** -- Matches any character NOT in the set
- `\d` / `\D` -- Digits/Non-digits
- `\s` / `\S` -- Whitespace/Non-whitespace
- `\w` -- Alphanumeric words (underscores are counted)
- `\\` -- Literal escape
- **CASE INSENSITIVITY** -- Toggle `is_not_case_sensitive` flag in `regen_t`.
- **BACKTRACKING** -- When a greedy match causes the rest of the pattern to fail, the engine steps back to find a valid path.

## LICENSE

This project is released under the GNU Affero General Public License version 3.0 or later.

**NO WARRANTY PROVIDED**

For full terms, see the `LICENSE` file in the project root or visit **https://www.gnu.org/licenses/agpl-3.0.en.html**.
