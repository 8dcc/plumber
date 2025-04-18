
#ifndef PATTERN_H_
#define PATTERN_H_ 1

#include <stdbool.h>

/*
 * Return true if string 'str' mathes regex pattern 'pat'.
 *
 * Pattern uses ERE syntax:
 * https://www.gnu.org/software/sed/manual/html_node/BRE-syntax.html
 *
 * TODO: Change parameter order.
 */
bool pattern_matches(const char* str, const char* pat);

/*
 * Try to find parenthesized 'group' of 'pat' in 'str'; write the 'start' and
 * 'end' of the group match. Returns true on success, or false otherwise.
 *
 * If the group number is negative, it's relative to the last match (-1 means
 * last matched group).
 */
bool pattern_find(const char* str,
                  const char* pat,
                  int group,
                  int* start,
                  int* end);

#endif /* PATTERN_H_ */
