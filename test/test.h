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

#ifndef TEST_H_
#define TEST_H_ 1

#include <stdio.h>  /* fprintf, fputc */
#include <stdlib.h> /* exit */

/*----------------------------------------------------------------------------*/
/* Generic testing functionality */

/*
 * This macro often expands to the basename of the input file. This problem
 * could also be solved with the '-fmacro-prefix-map=FOO=BAR' option.
 *
 * Since the '__FILE_NAME__' macro is not guaranteed to be defined, we fallback
 * to '__FILE__'.
 */
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif

/*
 * Show context information, error message, and a newline; exit with error
 * code 1.
 */
#define TEST_DIE(...)                                                          \
    do {                                                                       \
        fprintf(stderr,                                                        \
                "[test] %s:%d (%s): ",                                         \
                __FILE_NAME__,                                                 \
                __LINE__,                                                      \
                __func__);                                                     \
        fprintf(stderr, __VA_ARGS__);                                          \
        fputc('\n', stderr);                                                   \
        exit(1);                                                               \
    } while (0)

/*
 * Test that the specified condition evaluates to a non-zero value at runtime.
 * If the test fails, an error will be shown, and the execution will stop.
 */
#define TEST_COND(COND)                                                        \
    do {                                                                       \
        if ((COND) == 0) {                                                     \
            TEST_DIE("Condition `%s' failed.", #COND);                         \
        }                                                                      \
    } while (0)

/*
 * If COND is zero at compile-time, stop.
 */
#define TEST_STATIC_COND(COND)                                                 \
    _Static_assert(COND, "Static condition `" #COND "' failed.")

/*
 * Assert that TYPEA is effectively equal to TYPEB.
 */
#define SL_ASSERT_TYPES(TYPEA, TYPEB)                                          \
    TEST_STATIC_COND(__builtin_types_compatible_p(TYPEA, TYPEB))

/*----------------------------------------------------------------------------*/
/* Project-specific testing */

#define TEST_PATTERN(PATTERN, STR)                                             \
    do {                                                                       \
        if (!pattern_matches(STR, PATTERN)) {                                  \
            TEST_DIE("String '%s' failed to match pattern '%s'.",              \
                     STR,                                                      \
                     PATTERN);                                                 \
        }                                                                      \
    } while (0)

#endif /* TEST_H_ */
