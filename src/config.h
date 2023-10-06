
/* Commands used for plumbing depending on the text type */
#define CMD_BROWSER "firefox"
#define CMD_PDF     "firefox"
#define CMD_IMAGE   "feh"
#define CMD_VIDEO   "mpv"

/* These programs will be launched in another st instance. See ST_LAUNCH() */
#define CMD_EDITOR "nvim"
#define CMD_MAN    "man"

/*----------------------------------------------------------------------------*/
/* Regex patterns */

#define REGEX_EXTENSION(STR) "^.+\\." STR "$"

#define REGEX_URL     "^http.*:\\/\\/.+\\..+"
#define REGEX_PDF     REGEX_EXTENSION("pdf")
#define REGEX_MAN     "^[a-zA-Z0-9_-]+\\([0-9]\\)$"
#define REGEX_LINENO  "^.+:[0-9]+$"
#define REGEX_LINECOL "^.+:[0-9]+:[0-9]+.*"

/* Regex patterns of filenames used with CMD_EDITOR */
const char* editor_patterns[] = {
    REGEX_EXTENSION("c"),    /**/
    REGEX_EXTENSION("h"),    /**/
    REGEX_EXTENSION("cpp"),  /**/
    REGEX_EXTENSION("hpp"),  /**/
    REGEX_EXTENSION("asm"),  /**/
    REGEX_EXTENSION("txt"),  /**/
    REGEX_EXTENSION("mk"),   /**/
    REGEX_EXTENSION("md"),   /**/
    REGEX_EXTENSION("org"),  /**/
    REGEX_EXTENSION("diff"), /**/
};

/* Regex patterns of filenames used with CMD_IMAGE */
const char* image_patterns[] = {
    REGEX_EXTENSION("png"),  /**/
    REGEX_EXTENSION("jpg"),  /**/
    REGEX_EXTENSION("jpeg"), /**/
    REGEX_EXTENSION("gif"),  /**/
};

/* Regex patterns of filenames used with CMD_VIDEO */
const char* video_patterns[] = {
    REGEX_EXTENSION("mp4"), /**/
    REGEX_EXTENSION("mkv"), /**/
    REGEX_EXTENSION("avi"), /**/
};
