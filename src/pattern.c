/*
 * Copyright 2025 8dcc
 *
 * This file is part of plumber.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "pattern.h"

#include <stdbool.h>
#include <stddef.h>
#include <regex.h>

#include "util.h"

#define MAX_REGEX_GROUPS 10

/*
 * Returns true if string `str' mathes regex pattern `pat'.
 *
 * Pattern uses ERE syntax:
 * https://www.gnu.org/software/sed/manual/html_node/BRE-syntax.html
 */
bool pattern_matches(const char* str, const char* pat) {
    static regex_t r;

    /* Compile regex pattern ignoring case */
    if (regcomp(&r, pat, REG_EXTENDED | REG_ICASE)) {
        ERR("regcomp returned an error code for pattern \"%s\"", pat);
        return false;
    }

    const int code = regexec(&r, str, 0, NULL, 0);
    regfree(&r);

    if (code > REG_NOMATCH) {
        char err[100];
        regerror(code, &r, err, sizeof(err));
        ERR("regexec returned an error: %s", err);
        return false;
    }

    /* REG_NOERROR: Success
     * REG_NOMATCH: Pattern did not match */
    return code == REG_NOERROR;
}

/*
 * Try to find parenthesized `group' of `pat' in `str'. Write the `start' and
 * `end' of the group match, and return true on success.
 *
 * If the group number is negative, it's relative to the last match (-1 means
 * last matched group).
 */
bool pattern_find(const char* str,
                  const char* pat,
                  int group,
                  int* start,
                  int* end) {
    static regex_t r;
    static regmatch_t pmatch[MAX_REGEX_GROUPS];

    /*
     * FIXME: Display regex errors with regerror(3).
     */

    /* Compile regex pattern ignoring case */
    if (regcomp(&r, pat, REG_EXTENDED | REG_ICASE)) {
        ERR("Could not compile pattern '%s'", pat);
        return false;
    }

    /*
     * Real number of groups, calculated by regcomp(). We add one because the
     * first element of `pmatch' is the global match.
     */
    const int nmatch = r.re_nsub + 1;
    if (nmatch > MAX_REGEX_GROUPS) {
        ERR("Regular expression exceeds the number of parenthesized groups "
            "(%d/%d)",
            nmatch,
            MAX_REGEX_GROUPS);
        regfree(&r);
        return false;
    }

    const int code = regexec(&r, str, nmatch, pmatch, 0);
    regfree(&r);

    if (code > REG_NOMATCH) {
        char err[100];
        regerror(code, &r, err, sizeof(err));
        ERR("regexec returned an error: %s", err);
        return false;
    }

    /* If the group number is negative, it's relative to the last match */
    if (group < 0)
        group += nmatch;

    *start = pmatch[group].rm_so;
    *end   = pmatch[group].rm_eo;
    return code == REG_NOERROR && *start >= 0 && *end >= 0;
}
