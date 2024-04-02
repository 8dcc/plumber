
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <regex.h>

#include "config.h"

/* Used to get the compile-time length of arrays */
#define LENGTH(ARR) (int)(sizeof(ARR) / sizeof((ARR)[0]))

/* Used to execute commands */
#define LAUNCH(CMD, ...) execlp(CMD, CMD, __VA_ARGS__, NULL)

/* Execute command from new st(1) instance */
#define ST_LAUNCH(...) execlp("st", "st", "-e", __VA_ARGS__, NULL)

/* Used for the output of "--help" */
#define HELP_LINE(STR, DESC, ...) \
    fprintf(stderr, "    %s %-20s - " DESC "\n", argv[0], STR, __VA_ARGS__)

/* Returns true if string `str' mathes regex pattern `pat'. Pattern uses ERE
 * syntax: https://www.gnu.org/software/sed/manual/html_node/BRE-syntax.html */
static bool regex(const char* str, const char* pat) {
    static regex_t r;

    /* Compile regex pattern ignoring case */
    if (regcomp(&r, pat, REG_EXTENDED | REG_ICASE)) {
        fprintf(stderr,
                "plumber: regex: regcomp returned an error code for pattern "
                "\"%s\"\n",
                pat);
        return false;
    }

    int code = regexec(&r, str, 0, NULL, 0);
    if (code > REG_NOMATCH) {
        char err[100];
        regerror(code, &r, err, sizeof(err));
        fprintf(stderr, "plumber: regex: regexec returned an error: %s\n", err);
        return false;
    }

    /* REG_NOERROR: Success
     * REG_NOMATCH: Pattern did not match */
    return code == REG_NOERROR;
}

/* Convert "file:line:col" (or just "file:line") to vim argument format:
 * "+call cursor(line,col)" */
static char* line_to_vim(char* str) {
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
    if (*str != ':') {
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
     * FIXME: Launch commands like "vim" and "man" inside the same shell as ST,
     * instead of the caller. This is a ST issue.
     */

    /*
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
        char* line_arg = line_to_vim(argv[1]);
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
