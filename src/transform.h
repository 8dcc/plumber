
#ifndef TRANSFORM_H_
#define TRANSFORM_H_ 1

/*
 * Convert "file:line:col" (or just "file:line") to vim argument format:
 * "+call cursor(line,col)"
 *
 * Modifies the 'str' argument, and returns a static string that should not be
 * freed by the caller.
 */
const char* transform_line_to_vim(char* str);

/*
 * Remove spaces, single quotes and double quotes of start and end string
 * in-place; return `str'.
 */
char* transform_trim_quotes(char* str);

#endif /* TRANSFORM_H_ */
