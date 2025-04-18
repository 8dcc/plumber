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

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "pattern.h"
#include "transform.h"
#include "util.h"
#include "config.h"

/* Used to execute commands */
#define LAUNCH(CMD, ...) execlp(CMD, CMD, __VA_ARGS__, NULL)

/* Execute command from new st(1) instance */
#define ST_LAUNCH(...) execlp("st", "st", "-e", __VA_ARGS__, NULL)

/* Used for the output of "--help" */
#define HELP_LINE(STR, DESC, ...)                                              \
    fprintf(stderr, "    %s %-20s - " DESC "\n", argv[0], STR, __VA_ARGS__)

/*----------------------------------------------------------------------------*/
/* Enums */

enum EExitCodes {
    EXITSUCCESS     = 0,
    EXITFAILURE     = 1, /* Command-line argument didn't match any regexp */
    EXITINVALIDARGS = 2, /* Command-line arguments were invalid */
    EXITHELP        = 3, /* The only command-line argument was '--help' */
};

/*----------------------------------------------------------------------------*/
/* Main function */

int main(int argc, char** argv) {
    /* Too few arguments. Just ignore. */
    if (argc <= 1)
        return EXITINVALIDARGS;

    if (argc == 2 && !strcmp(argv[1], "--help")) {
        fprintf(stderr,
                "Usage: %s [REGEXP]\n"
                "Examples:\n",
                argv[0]);
        HELP_LINE("https://example.com", "Open in browser (%s)", CMD_BROWSER);
        HELP_LINE("file.pdf", "Open in PDF viewer (%s)", CMD_PDF);
        HELP_LINE("cmd(1)", "Open man page (%s)", CMD_MAN);
        HELP_LINE("image.png", "Open in image viewer (%s)", CMD_IMAGE);
        HELP_LINE("video.mkv", "Open in video player (%s)", CMD_VIDEO);
        HELP_LINE("file.txt", "Open in text editor (%s)", CMD_EDITOR);
        HELP_LINE("source.c:13:5", "Open at line and column (%s)", CMD_EDITOR);
        return EXITHELP;
    }

    /*
     * If the argument contains quotes in the start or end of the string, trim
     * them in-place.
     */
    transform_trim_quotes(argv[1]);

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
    if (pattern_matches(argv[1], REGEX_URL))
        return LAUNCH(CMD_BROWSER, argv[1]);

    /* PDF files */
    if (pattern_matches(argv[1], REGEX_PDF))
        return LAUNCH(CMD_PDF, argv[1]);

    /* Man pages */
    if (pattern_matches(argv[1], REGEX_MAN))
        return ST_LAUNCH(CMD_MAN, argv[1]);

    /* Filenames with line and col number (e.g. compiler errors)
     * Filenames with just line number (e.g. grep output) */
    if (pattern_matches(argv[1], REGEX_LINECOL)) {
        const char* line_arg = transform_line_to_vim(argv[1]);
        return ST_LAUNCH(CMD_EDITOR, argv[1], line_arg);
    }

    /* Iterate file extensions that should be opened with an image viewer */
    for (int i = 0; i < LENGTH(image_patterns); i++)
        if (pattern_matches(argv[1], image_patterns[i]))
            return LAUNCH(CMD_IMAGE, argv[1]);

    /* Iterate file extensions that should be opened with an video player */
    for (int i = 0; i < LENGTH(video_patterns); i++)
        if (pattern_matches(argv[1], video_patterns[i]))
            return LAUNCH(CMD_VIDEO, argv[1]);

    /* Iterate file extensions that should be opened with a text editor */
    for (int i = 0; i < LENGTH(editor_patterns); i++)
        if (pattern_matches(argv[1], editor_patterns[i]))
            return ST_LAUNCH(CMD_EDITOR, argv[1]);

#ifdef DEBUG
    ERR("Invalid pattern. Dumping arguments...");
    for (int i = 1; i < argc; i++)
        fprintf(stderr, "[%d] '%s'\n", i, argv[i]);
#endif

    return EXITFAILURE;
}
