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

#include "transform.h"

#include <string.h>
#include <ctype.h>

#include "pattern.h" /* pattern_find() */

/*----------------------------------------------------------------------------*/

/*
 * These will be used by 'transform_trim_quotes' to remove quotes from the start
 * and end of its input.
 */
#define REGEX_QUOTE       "(\'|\")"
#define REGEX_QUOTE_START "^\\s*" REGEX_QUOTE
#define REGEX_QUOTE_END   REGEX_QUOTE "\\s*$"

/*----------------------------------------------------------------------------*/

const char* transform_line_to_vim(char* str) {
    static char ret[] = "+call cursor(999999,999999)";
    char* pLine       = &ret[13];

    /* Skip "file:" */
    while (*str != ':')
        str++;

    /* End filename string where the ':' start */
    *str++ = '\0';

    /* Save "line" part */
    while (isdigit(*str))
        *pLine++ = *str++;

    /* If there is no colon after line, there is no column; just use 0 */
    if (*str != ':' || !isdigit(str[1])) {
        strcpy(pLine, ",0)");
        return ret;
    }

    *pLine++ = ',';
    str++;

    /* No digit after the line number (e.g. regex outputs) */
    if (!isdigit(*str)) {
        strcpy(pLine, "0)");
        return ret;
    }

    /* Save "col" part */
    while (isdigit(*str))
        *pLine++ = *str++;

    /* Close string */
    *pLine++ = ')';
    *pLine++ = '\0';

    return ret;
}

char* transform_trim_quotes(char* str) {
    /* Find the real start of the string */
    int prefix_start, prefix_end;
    if (pattern_find(str, REGEX_QUOTE_START, 1, &prefix_start, &prefix_end)) {
        /* Shift string 'prefix_end' characters */
        for (int i = 0; str[i] != '\0'; i++)
            str[i] = str[i + prefix_end];
    }

    /* Find the real end of the string */
    int subfix_start, subfix_end;
    if (pattern_find(str, REGEX_QUOTE_END, -1, &subfix_start, &subfix_end)) {
        /* End string at 'subfix_start' */
        str[subfix_start] = '\0';
    }

    return str;
}
