
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

    /* NOTE: From here, test the regex of the arguments and launch the asociated
     * program. The ones checked first obviously have more priority, so they
     * should be the most specific ones. */

    /* TODO:
     *  - man(1) pages
     *  - compiler errors
     *  - jump to line and col when launching editor (file.txt:5) */

    /* "http?://?.?" */
    if (regex(argv[1], "^http.*:\\/\\/.+\\..+"))
        return LAUNCH(CMD_BROWSER, argv[1]);

    /* "*.pdf" */
    if (regex(argv[1], "^.+\\.pdf$"))
        return LAUNCH(CMD_PDF, argv[1]);

    /* FIXME: In ST, the commands are launched from the CALLER of ST (dwm,
     * another term, etc.), so the changes are not reflected in the current ST.
     * This is an issue with the ST patch */

    /* Iterate file extensions that should be opened with a text editor */
    for (int i = 0; i < LENGTH(editor_patterns); i++)
        if (regex(argv[1], editor_patterns[i]))
            return LAUNCH(CMD_EDITOR, argv[1]);

#ifdef DEBUG
    fprintf(stderr, "plumber: Invalid pattern:\n");
    for (int i = 1; i < argc; i++)
        fprintf(stderr, "[%d]: %s\n", i, argv[i]);
#endif

    return 0;
}
