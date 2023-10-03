
/* Used to execute commands */
#define LAUNCH(cmd, ...) execlp(cmd, cmd, __VA_ARGS__, NULL)

/* For plumbing depending on the text type */
#define CMD_BROWSER "firefox"
#define CMD_PDF     "firefox"
