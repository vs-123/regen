# REGEN

Regen is a dead-simple regex-engine written in C99. The term `regen` is a syllabic abbreviation of **reg**ex-**en**gine.

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
