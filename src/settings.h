
/* Commands used for plumbing depending on the text type */
#define CMD_BROWSER "firefox"
#define CMD_PDF     "firefox"
#define CMD_EDITOR  "nvim"

#define REGEX_EXTENSION(STR) "^.+\\." STR "$"

/* Regex patterns of filenames used with CMD_EDITOR */
static const char* editor_patterns[] = {
    REGEX_EXTENSION("c"),   /**/
    REGEX_EXTENSION("h"),   /**/
    REGEX_EXTENSION("cpp"), /**/
    REGEX_EXTENSION("hpp"), /**/
    REGEX_EXTENSION("asm"), /**/
    REGEX_EXTENSION("txt"), /**/
    REGEX_EXTENSION("mk"),  /**/
    REGEX_EXTENSION("md"),  /**/
};
