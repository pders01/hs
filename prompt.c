#include "prompt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

shell_t
shell_detect(const char *arg)
{
	if (arg) {
		if (strcmp(arg, "bash") == 0) return SHELL_BASH;
		if (strcmp(arg, "zsh") == 0)  return SHELL_ZSH;
	}
	const char *sh = getenv("SHELL");
	if (sh) {
		if (strstr(sh, "bash")) return SHELL_BASH;
		if (strstr(sh, "zsh"))  return SHELL_ZSH;
	}
	return SHELL_UNKNOWN;
}

void
prompt_init(prompt_buf_t *pb, shell_t shell)
{
	pb->len = 0;
	pb->buf[0] = '\0';
	pb->shell = shell;
}

static void
buf_cat(prompt_buf_t *pb, const char *s, size_t n)
{
	if (pb->len + n >= HS_PROMPT_BUF_SIZE - 1)
		n = HS_PROMPT_BUF_SIZE - 1 - pb->len;
	if (n == 0)
		return;
	memcpy(pb->buf + pb->len, s, n);
	pb->len += n;
	pb->buf[pb->len] = '\0';
}

void
prompt_append(prompt_buf_t *pb, const char *s)
{
	buf_cat(pb, s, strlen(s));
}

void
prompt_color(prompt_buf_t *pb, const char *code)
{
	/*
	 * Wrap ANSI escapes in shell-specific non-printing markers so the
	 * shell can correctly calculate visible prompt width.
	 *
	 * bash (programmatic PS1): \x01 ... \x02  (raw SOH/STX)
	 * zsh:                     %{ ... %}
	 */
	char esc[64];
	int n = snprintf(esc, sizeof(esc), "\033[%sm", code);
	if (n < 0 || (size_t)n >= sizeof(esc))
		return;

	switch (pb->shell) {
	case SHELL_BASH:
		buf_cat(pb, "\x01", 1);
		buf_cat(pb, esc, (size_t)n);
		buf_cat(pb, "\x02", 1);
		break;
	case SHELL_ZSH:
		buf_cat(pb, "%{", 2);
		buf_cat(pb, esc, (size_t)n);
		buf_cat(pb, "%}", 2);
		break;
	default:
		buf_cat(pb, esc, (size_t)n);
		break;
	}
}

void
prompt_reset(prompt_buf_t *pb)
{
	prompt_color(pb, "0");
}

void
prompt_flush(prompt_buf_t *pb)
{
	fwrite(pb->buf, 1, pb->len, stdout);
}
