#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <pwd.h>

#include "config.h"
#include "prompt.h"
#include "git.h"

/* ── Shell init scripts ───────────────────────────────────────────── */

static void
cmd_init_bash(void)
{
	puts(
		"__hs_start=\n"
		"__hs_preexec() {\n"
		"    __hs_start=\"${EPOCHREALTIME:-}\"\n"
		"}\n"
		"__hs_precmd() {\n"
		"    local e=$?\n"
		"    local d=0\n"
		"    if [ -n \"$__hs_start\" ]; then\n"
		"        local now=\"${EPOCHREALTIME:-}\"\n"
		"        if [ -n \"$now\" ]; then\n"
		"            d=$(printf '%s\\n' \"$__hs_start $now\" | awk '{printf \"%d\", ($2-$1)*1000}')\n"
		"        fi\n"
		"    fi\n"
		"    __hs_start=\n"
		"    local j; j=$(jobs -p 2>/dev/null | wc -l)\n"
	"    PS1=\"$(hs prompt --exit-code=$e --duration=$d --jobs=$j --shell=bash)\"\n"
		"}\n"
		"trap '__hs_preexec' DEBUG\n"
		"PROMPT_COMMAND=\"__hs_precmd${PROMPT_COMMAND:+;$PROMPT_COMMAND}\""
	);
}

static void
cmd_init_zsh(void)
{
	puts(
		"typeset -g __hs_start\n"
		"__hs_preexec() { __hs_start=$EPOCHREALTIME }\n"
		"__hs_precmd() {\n"
		"    local e=$?\n"
		"    local d=0\n"
		"    if [[ -n $__hs_start ]]; then\n"
		"        d=$(( (EPOCHREALTIME - __hs_start) * 1000 ))\n"
		"        d=${d%%.*}\n"
		"    fi\n"
		"    __hs_start=\n"
		"    local j=${(%):-%j}\n"
	"    PROMPT=\"$(hs prompt --exit-code=$e --duration=$d --jobs=$j --shell=zsh)\"\n"
		"}\n"
		"precmd_functions+=(__hs_precmd)\n"
		"preexec_functions+=(__hs_preexec)"
	);
}

static int
cmd_init(const char *shell)
{
	if (!shell) {
		fprintf(stderr, "usage: hs init <bash|zsh>\n");
		return 1;
	}
	if (strcmp(shell, "bash") == 0)      cmd_init_bash();
	else if (strcmp(shell, "zsh") == 0)  cmd_init_zsh();
	else {
		fprintf(stderr, "hs: unsupported shell '%s'\n", shell);
		return 1;
	}
	return 0;
}

/* ── Directory shortening ─────────────────────────────────────────── */

static const char *
shorten_dir(const char *cwd, char *out, size_t outsz)
{
	/* Replace $HOME prefix with ~ */
	const char *home = getenv("HOME");
	if (!home) {
		struct passwd *pw = getpwuid(getuid());
		if (pw) home = pw->pw_dir;
	}

	const char *dir = cwd;
	int tilde = 0;

	if (home && strncmp(cwd, home, strlen(home)) == 0 &&
	    (cwd[strlen(home)] == '/' || cwd[strlen(home)] == '\0')) {
		dir = cwd + strlen(home);
		tilde = 1;
	}

	/* Count components from the end, keep last HS_DIR_MAX_DEPTH */
	int nslash = 0;
	const char *p = dir + strlen(dir);
	while (p > dir) {
		p--;
		if (*p == '/' && ++nslash == HS_DIR_MAX_DEPTH)
			break;
	}

	/* If we're at home root exactly */
	if (tilde && *dir == '\0') {
		snprintf(out, outsz, "~");
		return out;
	}

	if (nslash >= HS_DIR_MAX_DEPTH && p > dir) {
		/* Truncated: show partial path */
		snprintf(out, outsz, "%s%s", tilde ? "~" : "", p);
	} else {
		snprintf(out, outsz, "%s%s", tilde ? "~" : "", dir);
	}

	return out;
}

/* ── Argument parsing ─────────────────────────────────────────────── */

static int
parse_exit_code(int argc, char **argv)
{
	for (int i = 2; i < argc; i++) {
		if (strncmp(argv[i], "--exit-code=", 12) == 0)
			return atoi(argv[i] + 12);
	}
	return 0;
}

static long
parse_duration(int argc, char **argv)
{
	for (int i = 2; i < argc; i++) {
		if (strncmp(argv[i], "--duration=", 11) == 0)
			return atol(argv[i] + 11);
	}
	return 0;
}

static int
parse_jobs(int argc, char **argv)
{
	for (int i = 2; i < argc; i++) {
		if (strncmp(argv[i], "--jobs=", 7) == 0)
			return atoi(argv[i] + 7);
	}
	return 0;
}

static const char *
parse_shell(int argc, char **argv)
{
	for (int i = 2; i < argc; i++) {
		if (strncmp(argv[i], "--shell=", 8) == 0)
			return argv[i] + 8;
	}
	return NULL;
}

/* ── Prompt command ───────────────────────────────────────────────── */

static int
cmd_prompt(int argc, char **argv)
{
	int exit_code     = parse_exit_code(argc, argv);
	long duration_ms  = parse_duration(argc, argv);
	int jobs          = parse_jobs(argc, argv);
	const char *shell = parse_shell(argc, argv);
	shell_t sh        = shell_detect(shell);

	prompt_buf_t pb;
	prompt_init(&pb, sh);

	/* Segment: user@host (SSH only) */
	int is_ssh = (getenv("SSH_CONNECTION") || getenv("SSH_TTY"));
	if (is_ssh) {
		char host[256];
		if (gethostname(host, sizeof(host)) != 0)
			snprintf(host, sizeof(host), "?");
		const char *user = getenv("USER");
		if (!user) {
			struct passwd *pw = getpwuid(getuid());
			user = pw ? pw->pw_name : "?";
		}
		prompt_color(&pb, HS_COLOR_SSH);
		prompt_append(&pb, user);
		prompt_append(&pb, "@");
		prompt_append(&pb, host);
		prompt_reset(&pb);
		prompt_append(&pb, " ");
	}

	/* Segment: exit code */
	if (exit_code != 0) {
		char tmp[16];
		snprintf(tmp, sizeof(tmp), "%s %d ", HS_SYM_FAILED, exit_code);
		prompt_color(&pb, HS_COLOR_ERR);
		prompt_append(&pb, tmp);
		prompt_reset(&pb);
	}

	/* Segment: directory */
	char cwd[PATH_MAX];
	if (!getcwd(cwd, sizeof(cwd)))
		snprintf(cwd, sizeof(cwd), "?");

	char dir[PATH_MAX];
	shorten_dir(cwd, dir, sizeof(dir));

	prompt_color(&pb, HS_COLOR_DIR);
	prompt_append(&pb, dir);
	prompt_reset(&pb);

	/* Segment: git */
	git_info_t gi;
	git_info_collect(&gi, cwd);

	if (gi.is_repo) {
		/* Branch */
		prompt_append(&pb, " ");
		prompt_color(&pb, HS_COLOR_DIM);
		prompt_append(&pb, "on ");
		prompt_reset(&pb);
		prompt_color(&pb, HS_COLOR_BRANCH);
		if (gi.detached) {
			prompt_append(&pb, ":");
			prompt_append(&pb, gi.branch);
		} else {
			prompt_append(&pb, gi.branch);
		}
		prompt_reset(&pb);

		/* State (merge, rebase, etc.) */
		const char *st = git_state_string(gi.state);
		if (st) {
			prompt_append(&pb, " ");
			prompt_color(&pb, HS_COLOR_STATE);
			prompt_append(&pb, st);
			prompt_reset(&pb);
		}

		/* Status indicators */
		int has_status = gi.staged || gi.modified ||
		                 gi.untracked || gi.conflicted;
		if (has_status) {
			prompt_append(&pb, " ");
			char tmp[32];
			if (gi.staged) {
				snprintf(tmp, sizeof(tmp), "%s%d",
					 HS_SYM_STAGED, gi.staged);
				prompt_color(&pb, HS_COLOR_STAGED);
				prompt_append(&pb, tmp);
				prompt_reset(&pb);
			}
			if (gi.modified) {
				if (gi.staged) prompt_append(&pb, " ");
				snprintf(tmp, sizeof(tmp), "%s%d",
					 HS_SYM_MODIFIED, gi.modified);
				prompt_color(&pb, HS_COLOR_MODIFIED);
				prompt_append(&pb, tmp);
				prompt_reset(&pb);
			}
			if (gi.untracked) {
				if (gi.staged || gi.modified)
					prompt_append(&pb, " ");
				snprintf(tmp, sizeof(tmp), "%s%d",
					 HS_SYM_UNTRACKED, gi.untracked);
				prompt_color(&pb, HS_COLOR_UNTRACKED);
				prompt_append(&pb, tmp);
				prompt_reset(&pb);
			}
			if (gi.conflicted) {
				if (gi.staged || gi.modified || gi.untracked)
					prompt_append(&pb, " ");
				snprintf(tmp, sizeof(tmp), "%s%d",
					 HS_SYM_CONFLICTED, gi.conflicted);
				prompt_color(&pb, HS_COLOR_CONFLICT);
				prompt_append(&pb, tmp);
				prompt_reset(&pb);
			}
		}

		/* Ahead / behind */
		if (gi.has_upstream && (gi.ahead || gi.behind)) {
			prompt_append(&pb, " ");
			char tmp[32];
			if (gi.ahead) {
				snprintf(tmp, sizeof(tmp), "%s%d",
					 HS_SYM_AHEAD, gi.ahead);
				prompt_color(&pb, HS_COLOR_AHEAD);
				prompt_append(&pb, tmp);
				prompt_reset(&pb);
			}
			if (gi.behind) {
				if (gi.ahead) prompt_append(&pb, " ");
				snprintf(tmp, sizeof(tmp), "%s%d",
					 HS_SYM_BEHIND, gi.behind);
				prompt_color(&pb, HS_COLOR_BEHIND);
				prompt_append(&pb, tmp);
				prompt_reset(&pb);
			}
		}

		/* Stash */
		if (gi.stash_count > 0) {
			prompt_append(&pb, " ");
			char tmp[32];
			snprintf(tmp, sizeof(tmp), "%s%d",
				 HS_SYM_STASH, gi.stash_count);
			prompt_color(&pb, HS_COLOR_STASH);
			prompt_append(&pb, tmp);
			prompt_reset(&pb);
		}
	}

	/* Segment: command duration */
	if (duration_ms >= HS_DURATION_MIN_MS) {
		prompt_append(&pb, " ");
		prompt_color(&pb, HS_COLOR_DURATION);
		prompt_append(&pb, HS_SYM_DURATION);
		char dur[32];
		if (duration_ms >= 60000) {
			long min = duration_ms / 60000;
			long sec = (duration_ms % 60000) / 1000;
			snprintf(dur, sizeof(dur), "%ldm%lds", min, sec);
		} else {
			long sec = duration_ms / 1000;
			snprintf(dur, sizeof(dur), "%lds", sec);
		}
		prompt_append(&pb, dur);
		prompt_reset(&pb);
	}

	/* Segment: background jobs */
	if (jobs > 0) {
		prompt_append(&pb, " ");
		prompt_color(&pb, HS_COLOR_JOBS);
		char tmp[16];
		snprintf(tmp, sizeof(tmp), "%s%d", HS_SYM_JOBS, jobs);
		prompt_append(&pb, tmp);
		prompt_reset(&pb);
	}

	/* Segment: prompt char (on new line) */
	prompt_append(&pb, "\n");
	prompt_color(&pb, exit_code == 0 ? HS_COLOR_OK : HS_COLOR_ERR);
	prompt_append(&pb, HS_SYM_PROMPT " ");
	prompt_reset(&pb);

	/* Agent mode: JSON output instead of prompt */
	const char *agent = getenv("HS_AGENT");
	if (agent && strcmp(agent, "1") == 0) {
		printf("{\"cwd\":\"%s\",\"exit_code\":%d,\"duration_ms\":%ld,\"jobs\":%d,\"ssh\":%s",
		       cwd, exit_code, duration_ms, jobs,
		       is_ssh ? "true" : "false");
		if (gi.is_repo) {
			const char *st = git_state_string(gi.state);
			printf(",\"git\":{\"branch\":\"%s\",\"detached\":%s,"
			       "\"dirty\":%s,\"staged\":%d,\"modified\":%d,"
			       "\"untracked\":%d,\"conflicted\":%d,"
			       "\"ahead\":%d,\"behind\":%d,"
			       "\"stash\":%d,\"state\":\"%s\"}",
			       gi.branch,
			       gi.detached ? "true" : "false",
			       (gi.staged || gi.modified || gi.untracked ||
			        gi.conflicted) ? "true" : "false",
			       gi.staged, gi.modified,
			       gi.untracked, gi.conflicted,
			       gi.ahead, gi.behind,
			       gi.stash_count,
			       st ? st : "none");
		}
		printf("}\n");
		git_info_cleanup(&gi);
		return 0;
	}

	prompt_flush(&pb);
	git_info_cleanup(&gi);
	return 0;
}

/* ── Main ─────────────────────────────────────────────────────────── */

static void
usage(void)
{
	fprintf(stderr,
		"hs — minimalist shell prompt\n"
		"\n"
		"usage:\n"
		"  hs init <bash|zsh>                      print shell init script\n"
		"  hs prompt [options]                       render prompt\n"
		"\n"
		"options:\n"
		"  --exit-code=N    last command exit code\n"
		"  --duration=MS    last command duration in milliseconds\n"
		"  --jobs=N         number of background jobs\n"
		"  --shell=SHELL    bash or zsh\n"
	);
}

int
main(int argc, char **argv)
{
	if (argc < 2) {
		usage();
		return 1;
	}

	if (strcmp(argv[1], "init") == 0)
		return cmd_init(argc > 2 ? argv[2] : NULL);

	if (strcmp(argv[1], "prompt") == 0)
		return cmd_prompt(argc, argv);

	usage();
	return 1;
}
