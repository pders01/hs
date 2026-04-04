#ifndef HS_PROMPT_H
#define HS_PROMPT_H

#include <stddef.h>
#include "config.h"

typedef enum {
	SHELL_BASH,
	SHELL_ZSH,
	SHELL_UNKNOWN
} shell_t;

typedef struct {
	char    buf[HS_PROMPT_BUF_SIZE];
	size_t  len;
	shell_t shell;
} prompt_buf_t;

/* Detect shell from string argument ("bash", "zsh") or $SHELL env. */
shell_t shell_detect(const char *arg);

/* Initialize prompt buffer. */
void prompt_init(prompt_buf_t *pb, shell_t shell);

/* Append raw string. */
void prompt_append(prompt_buf_t *pb, const char *s);

/* Set foreground color. `code` is the ANSI body, e.g. "1;36". */
void prompt_color(prompt_buf_t *pb, const char *code);

/* Reset all attributes. */
void prompt_reset(prompt_buf_t *pb);

/* Write buffer contents to stdout. */
void prompt_flush(prompt_buf_t *pb);

#endif /* HS_PROMPT_H */
