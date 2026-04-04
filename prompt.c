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
	 * Write the entire wrapped sequence atomically to prevent split
	 * markers (e.g. \x01 without \x02) on buffer truncation.
	 *
	 * bash (programmatic PS1): \x01 ... \x02  (raw SOH/STX)
	 * zsh:                     %{ ... %}
	 */
	char wrapped[80];
	int n;

	switch (pb->shell) {
	case SHELL_BASH:
		n = snprintf(wrapped, sizeof(wrapped),
			     "\x01\033[%sm\x02", code);
		break;
	case SHELL_ZSH:
		n = snprintf(wrapped, sizeof(wrapped),
			     "%%{\033[%sm%%}", code);
		break;
	default:
		n = snprintf(wrapped, sizeof(wrapped),
			     "\033[%sm", code);
		break;
	}

	if (n > 0 && (size_t)n < sizeof(wrapped))
		buf_cat(pb, wrapped, (size_t)n);
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
