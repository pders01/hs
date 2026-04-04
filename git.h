#ifndef HS_GIT_H
#define HS_GIT_H

#include "config.h"

typedef struct {
	int  is_repo;
	int  detached;
	int  state;                      /* git_repository_state_t */
	char branch[HS_BRANCH_MAX];

	/* status counts */
	int  staged;
	int  modified;
	int  untracked;
	int  conflicted;

	/* upstream */
	int  has_upstream;
	int  ahead;
	int  behind;

	/* stash */
	int  stash_count;
} git_info_t;

/* Return human-readable string for repo state, or NULL if none. */
const char *git_state_string(int state);

/* Collect all git info for the given working directory.
 * Caller must call git_info_cleanup() when done. */
void git_info_collect(git_info_t *info, const char *cwd);

/* Release any resources. Safe to call on a zeroed struct. */
void git_info_cleanup(git_info_t *info);

#endif /* HS_GIT_H */
