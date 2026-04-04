#include "git.h"
#include <stdio.h>
#include <string.h>
#include <git2.h>

/* ── Helpers ──────────────────────────────────────────────────────── */

const char *
git_state_string(int st)
{
	switch (st) {
	case GIT_REPOSITORY_STATE_MERGE:                   return "MERGING";
	case GIT_REPOSITORY_STATE_REVERT:                  return "REVERTING";
	case GIT_REPOSITORY_STATE_REVERT_SEQUENCE:         return "REVERTING";
	case GIT_REPOSITORY_STATE_CHERRYPICK:              return "CHERRY-PICKING";
	case GIT_REPOSITORY_STATE_CHERRYPICK_SEQUENCE:     return "CHERRY-PICKING";
	case GIT_REPOSITORY_STATE_BISECT:                  return "BISECTING";
	case GIT_REPOSITORY_STATE_REBASE:                  return "REBASING";
	case GIT_REPOSITORY_STATE_REBASE_INTERACTIVE:      return "REBASING";
	case GIT_REPOSITORY_STATE_REBASE_MERGE:            return "REBASING";
	case GIT_REPOSITORY_STATE_APPLY_MAILBOX:           return "APPLYING";
	case GIT_REPOSITORY_STATE_APPLY_MAILBOX_OR_REBASE: return "APPLYING";
	default:                                           return NULL;
	}
}

#if HS_GIT_STASH
static int
stash_counter_cb(size_t idx, const char *msg, const git_oid *oid, void *payload)
{
	(void)idx; (void)msg; (void)oid;
	(*(int *)payload)++;
	return 0;
}
#endif

/* ── Public API ───────────────────────────────────────────────────── */

void
git_info_collect(git_info_t *info, const char *cwd)
{
	memset(info, 0, sizeof(*info));

	git_libgit2_init();

	git_repository *repo = NULL;
	if (git_repository_open_ext(&repo, cwd, 0, NULL) != 0)
		return;

	info->is_repo = 1;

	/* ── Branch ───────────────────────────────────────────────── */
	git_reference *head = NULL;
	if (git_repository_head(&head, repo) == 0) {
		if (git_repository_head_detached(repo)) {
			info->detached = 1;
			const git_oid *oid = git_reference_target(head);
			if (oid)
				git_oid_tostr(info->branch, 8, oid); /* 7-char SHA */
		} else {
			const char *name = git_reference_shorthand(head);
			if (name)
				snprintf(info->branch, sizeof(info->branch), "%s", name);
		}
	} else {
		/* Unborn branch (empty repo) */
		git_reference *ref = NULL;
		if (git_reference_lookup(&ref, repo, "HEAD") == 0) {
			const char *target = git_reference_symbolic_target(ref);
			if (target) {
				const char *slash = strrchr(target, '/');
				snprintf(info->branch, sizeof(info->branch),
					 "%s", slash ? slash + 1 : target);
			}
			git_reference_free(ref);
		}
	}

	/* ── State ────────────────────────────────────────────────── */
	info->state = git_repository_state(repo);

	/* ── Status counts ────────────────────────────────────────── */
#if HS_GIT_STATUS
	{
		git_status_options opts = {0};
		opts.version = GIT_STATUS_OPTIONS_VERSION;
		opts.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
		opts.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED
		           | GIT_STATUS_OPT_EXCLUDE_SUBMODULES;

		git_status_list *sl = NULL;
		if (git_status_list_new(&sl, repo, &opts) == 0) {
			size_t n = git_status_list_entrycount(sl);
			for (size_t i = 0; i < n; i++) {
				const git_status_entry *e = git_status_byindex(sl, i);
				unsigned s = e->status;

				if (s & (GIT_STATUS_INDEX_NEW |
				         GIT_STATUS_INDEX_MODIFIED |
				         GIT_STATUS_INDEX_DELETED |
				         GIT_STATUS_INDEX_RENAMED |
				         GIT_STATUS_INDEX_TYPECHANGE))
					info->staged++;

				if (s & (GIT_STATUS_WT_MODIFIED |
				         GIT_STATUS_WT_DELETED |
				         GIT_STATUS_WT_TYPECHANGE |
				         GIT_STATUS_WT_RENAMED))
					info->modified++;

				if (s & GIT_STATUS_WT_NEW)
					info->untracked++;

				if (s & GIT_STATUS_CONFLICTED)
					info->conflicted++;
			}
			git_status_list_free(sl);
		}
	}
#endif

	/* ── Ahead / behind ───────────────────────────────────────── */
#if HS_GIT_AHEAD_BEHIND
	if (head && !info->detached) {
		git_reference *upstream = NULL;
		if (git_branch_upstream(&upstream, head) == 0) {
			info->has_upstream = 1;
			const git_oid *local_oid = git_reference_target(head);
			const git_oid *up_oid    = git_reference_target(upstream);
			if (local_oid && up_oid) {
				size_t a = 0, b = 0;
				if (git_graph_ahead_behind(&a, &b, repo,
				                           local_oid, up_oid) == 0) {
					info->ahead  = (int)a;
					info->behind = (int)b;
				}
			}
			git_reference_free(upstream);
		}
	}
#endif

	/* ── Stash count ──────────────────────────────────────────── */
#if HS_GIT_STASH
	git_stash_foreach(repo, stash_counter_cb, &info->stash_count);
#endif

	/* ── Cleanup ──────────────────────────────────────────────── */
	if (head)
		git_reference_free(head);
	git_repository_free(repo);

}

void
git_info_cleanup(git_info_t *info)
{
	git_libgit2_shutdown();
	memset(info, 0, sizeof(*info));
}
