
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <regex.h>

#include "config.h"

#define MAX_REGEX_GROUPS 10

/* Used to get the compile-time length of arrays */
#define LENGTH(ARR) (int)(sizeof(ARR) / sizeof((ARR)[0]))

/* Used to execute commands */
#define LAUNCH(CMD, ...) execlp(CMD, CMD, __VA_ARGS__, NULL)

/* Execute command from new st(1) instance */
#define ST_LAUNCH(...) execlp("st", "st", "-e", __VA_ARGS__, NULL)

/* Used for the output of "--help" */
#define HELP_LINE(STR, DESC, ...)                                              \
    fprintf(stderr, "    %s %-20s -  " DESC "\n", argv[0], STR, __VA_ARGS__)

#define ERR(...)                                                               \
    do {                                                                       \
        fprintf(stderr, "plumber: %s: ", __func__);                            \
        fprintf(stderr, __VA_ARGS__);                                          \
        fputc('\n', stderr);                                                   \
    } while (0)

/*----------------------------------------------------------------------------*/
/* Regex functions */

/*
 * Returns true if string `str' mathes regex pattern `pat'.
 *
 * Pattern uses ERE syntax:
 * https://www.gnu.org/software/sed/manual/html_node/BRE-syntax.html
 */
static bool regex(const char* str, const char* pat) {
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
static bool regex_find(const char* str, const char* pat, int group, int* start,
                       int* end) {
    static regex_t r;
    static regmatch_t pmatch[MAX_REGEX_GROUPS];

    /* Compile regex pattern ignoring case */
    if (regcomp(&r, pat, REG_EXTENDED | REG_ICASE)) {
        ERR("regcomp returned an error code for pattern \"%s\"", pat);
        return false;
    }

    /*
     * Real number of groups, calculated by regcomp(). We add one because the
     * first element of `pmatch' is the global match.
     */
    const int nmatch = r.re_nsub + 1;
    if (nmatch > MAX_REGEX_GROUPS) {
        ERR("regular expression exceeds the number of parenthesized groups "
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

/*----------------------------------------------------------------------------*/
/* String manipulation functions */

/*
 * Convert "file:line:col" (or just "file:line") to vim argument format:
 * "+call cursor(line,col)"
 *
 * Returns a static string that should not be freed by the caller.
 */
static const char* line_to_vim(char* str) {
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

    /* If there is no colon after line, there is no column. Just use 0 */
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

/*
 * Remove spaces, single quotes and double quotes of start and end string
 * in-place. Return `str'.
 */
static char* trim_quotes(char* str) {
    /* Find the real start of the string */
    int prefix_start, prefix_end;
    if (regex_find(str, REGEX_QUOTE_START, 1, &prefix_start, &prefix_end)) {
        /* Shift string `prefix_end' characters */
        for (int i = 0; str[i] != '\0'; i++)
            str[i] = str[i + prefix_end];
    }

    /* Find the real end of the string */
    int subfix_start, subfix_end;
    if (regex_find(str, REGEX_QUOTE_END, -1, &subfix_start, &subfix_end)) {
        /* End string at `subfix_start' */
        str[subfix_start] = '\0';
    }

    return str;
}

/*----------------------------------------------------------------------------*/
/* Main function */

int main(int argc, char** argv) {
    /* Too few arguments. Just ignore. */
    if (argc <= 1)
        return 0;

    if (argc == 2 && !strcmp(argv[1], "--help")) {
        fprintf(stderr, "Plumber usage:\n");
        HELP_LINE("https://example.com", "Open in browser (%s)", CMD_BROWSER);
        HELP_LINE("file.pdf", "Open in PDF viewer (%s)", CMD_PDF);
        HELP_LINE("cmd(1)", "Open man page (%s)", CMD_MAN);
        HELP_LINE("image.png", "Open in image viewer (%s)", CMD_IMAGE);
        HELP_LINE("video.mkv", "Open in video player (%s)", CMD_VIDEO);
        HELP_LINE("file.txt", "Open in text editor (%s)", CMD_EDITOR);
        HELP_LINE("source.c:13:5", "Open at line and column (%s)", CMD_EDITOR);
        return 0;
    }

    /*
     * If the argument contains quotes in the start or end of the string, trim
     * them in-place.
     */
    trim_quotes(argv[1]);

    /*
     * FIXME: Launch commands like "vim" and "man" inside the same shell as ST,
     * instead of the caller. This is a ST issue.
     *
     * NOTE: From here, test the regex of the arguments and launch the asociated
     * program. The ones checked first obviously have more priority, so they
     * should be the most specific ones.
     *
     * NOTE: Most macros use LAUNCH to simply run the command, but others use
     * the ST_LAUNCH macro, which opens another st(1) instance
     */

    /* URLs */
    if (regex(argv[1], REGEX_URL))
        return LAUNCH(CMD_BROWSER, argv[1]);

    /* PDF files */
    if (regex(argv[1], REGEX_PDF))
        return LAUNCH(CMD_PDF, argv[1]);

    /* Man pages */
    if (regex(argv[1], REGEX_MAN))
        return ST_LAUNCH(CMD_MAN, argv[1]);

    /* Filenames with line and col number (e.g. compiler errors)
     * Filenames with just line number (e.g. grep output) */
    if (regex(argv[1], REGEX_LINECOL)) {
        const char* line_arg = line_to_vim(argv[1]);
        return ST_LAUNCH(CMD_EDITOR, argv[1], line_arg);
    }

    /* Iterate file extensions that should be opened with an image viewer */
    for (int i = 0; i < LENGTH(image_patterns); i++)
        if (regex(argv[1], image_patterns[i]))
            return LAUNCH(CMD_IMAGE, argv[1]);

    /* Iterate file extensions that should be opened with an video player */
    for (int i = 0; i < LENGTH(video_patterns); i++)
        if (regex(argv[1], video_patterns[i]))
            return LAUNCH(CMD_VIDEO, argv[1]);

    /* Iterate file extensions that should be opened with a text editor */
    for (int i = 0; i < LENGTH(editor_patterns); i++)
        if (regex(argv[1], editor_patterns[i]))
            return ST_LAUNCH(CMD_EDITOR, argv[1]);

#ifdef DEBUG
    fprintf(stderr, "plumber: Invalid pattern:\n");
    for (int i = 1; i < argc; i++)
        fprintf(stderr, "[%d]: %s\n", i, argv[i]);
#endif

    return 0;
}
