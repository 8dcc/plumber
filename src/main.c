
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>

#include "settings.h"

/* Used to get the compile-time length of arrays */
#define LENGTH(ARR) (int)(sizeof(ARR) / sizeof((ARR)[0]))

/* Used to execute commands */
#define LAUNCH(CMD, ...) execlp(CMD, CMD, __VA_ARGS__, NULL)

/* Execute command from new st(1) instance */
#define ST_LAUNCH(...) execlp("st", "st", "-e", __VA_ARGS__, NULL)

/* Used for the output of "--help" */
#define HELP_LINE(STR, DESC, ...) \
    fprintf(stderr, "    %s " STR "\t- " DESC "\n", argv[0], __VA_ARGS__)

/* Returns true if string `str` mathes regex pattern `pat`. Pattern uses BRE
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

int main(int argc, char** argv) {
    /* Too few arguments. Just ignore. */
    if (argc <= 1)
        return 0;

    if (argc == 2 && !strcmp(argv[1], "--help")) {
        fprintf(stderr, "Plumber usage:\n");
        HELP_LINE("<URL>", "Open in browser (%s)", CMD_BROWSER);
        HELP_LINE("<PDF>", "Open in PDF viewer (%s)", CMD_PDF);
        return 0;
    }

    /*
     * TODO:
     *   - man(1) pages
     *   - Jump to line and col (file:line:col)
     *   - Compiler errors (path:line:col:*)
     */

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
