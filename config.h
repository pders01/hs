#ifndef HS_CONFIG_H
#define HS_CONFIG_H

/* ── Prompt layout ────────────────────────────────────────────────── */
#define HS_DIR_MAX_DEPTH    3       /* max path components to show    */
#define HS_BRANCH_MAX       128     /* max branch name length         */
#define HS_PROMPT_BUF_SIZE  2048    /* stack buffer for prompt string */

/* ── Thresholds ───────────────────────────────────────────────────── */
#define HS_DURATION_MIN_MS  2000    /* show duration above this (ms)  */

/* ── Symbols ──────────────────────────────────────────────────────── */
#define HS_SYM_PROMPT       "❯"
#define HS_SYM_FAILED       "✘"
#define HS_SYM_STAGED       "+"
#define HS_SYM_MODIFIED     "!"
#define HS_SYM_UNTRACKED    "?"
#define HS_SYM_CONFLICTED   "×"
#define HS_SYM_AHEAD        "↑"
#define HS_SYM_BEHIND       "↓"
#define HS_SYM_STASH        "≡"
#define HS_SYM_DURATION     "took "
#define HS_SYM_JOBS         "✦"

/* ── Colors (ANSI escape bodies, without \033[ prefix) ────────────── */
#define HS_COLOR_DIR        "1;36"  /* bold cyan                      */
#define HS_COLOR_BRANCH     "1;35"  /* bold magenta                   */
#define HS_COLOR_STAGED     "1;32"  /* bold green                     */
#define HS_COLOR_MODIFIED   "1;33"  /* bold yellow                    */
#define HS_COLOR_UNTRACKED  "1;34"  /* bold blue                      */
#define HS_COLOR_CONFLICT   "1;31"  /* bold red                       */
#define HS_COLOR_AHEAD      "1;32"  /* bold green                     */
#define HS_COLOR_BEHIND     "1;31"  /* bold red                       */
#define HS_COLOR_STATE      "1;33"  /* bold yellow                    */
#define HS_COLOR_STASH      "1;34"  /* bold blue                      */
#define HS_COLOR_OK         "1;32"  /* bold green                     */
#define HS_COLOR_ERR        "1;31"  /* bold red                       */
#define HS_COLOR_SSH        "1;33"  /* bold yellow                    */
#define HS_COLOR_DIM        "2"     /* dim/faint                      */
#define HS_COLOR_DURATION   "1;33"  /* bold yellow                    */
#define HS_COLOR_JOBS       "1;34"  /* bold blue                      */

/* ── Feature toggles ──────────────────────────────────────────────── */
#define HS_GIT_STATUS       1       /* enable git status counts       */
#define HS_GIT_AHEAD_BEHIND 1       /* enable ahead/behind upstream   */
#define HS_GIT_STASH        1       /* enable stash count             */

#endif /* HS_CONFIG_H */
