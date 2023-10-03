
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>

#include "settings.h" /* Only include once here */

#define HELP_LINE(STR, DESC, ...) \
    fprintf(stderr, "    %s " STR "\t- " DESC "\n", argv[0], __VA_ARGS__)

static bool regex(const char* str, const char* pat) {
    static regex_t r;

    if (regcomp(&r, pat, REG_ICASE)) {
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
        fprintf(stderr, "Usage:\n");
        HELP_LINE("<URL>", "Open in browser (%s)", CMD_BROWSER);
        HELP_LINE("<PDF>", "Open in PDF viewer (%s)", CMD_PDF);
        return 0;
    }

    /* FIXME: ".pdf" and "https://." is recogniced, ".+" doesn't work */

    /* "http?://?.?" */
    if (regex(argv[1], "^http.*:\\/\\/.*\\..*"))
        return LAUNCH(CMD_BROWSER, argv[1]);

    /* "*.pdf" */
    if (regex(argv[1], "^.*\\.pdf$"))
        return LAUNCH(CMD_PDF, argv[1]);

    return 0;
}
