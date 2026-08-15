# webserv — Team Workflow

This document describes the full workflow for working on this project, from picking up an issue to merging finished work into `main`. Follow it for every change, no matter how small.

**Status legend used below:**
- ✅ Works today with plain Git/GitHub — no setup needed
- 🔧 Requires setup we haven't built yet — see "CI & Automation Status" section at the bottom for current progress

---

## 0. One-time setup (per person, once)

```bash
git clone git@github.com:casgomp/webserv.git
cd webserv
git config user.name  "Your Name"
git config user.email "you@example.com"
```

Make sure you have push access to the repo and are added as a collaborator, and that you're a member of the linked GitHub Project (Kanban board).

---

## 1. Picking up work — Issues & the Kanban board ✅

1. Go to the repo's **Issues** tab (or the linked **Project** board directly).
2. If the task doesn't have an issue yet, create one:
   - Clear, action-oriented title (e.g. "Parse HTTP request headers", not "headers")
   - Description: what needs to be done
   - **Acceptance criteria**: a short checklist (`- [ ]` items) describing exactly how we'll know it's done
3. Assign yourself to the issue.
4. On the Kanban board, move the issue card from **Todo → In Progress**.
5. Note the issue number (e.g. `#12`) — you'll need it for your branch and commits.

---

## 2. Branch naming convention ✅

Create a branch **from an up-to-date `main`**, named to describe the work:

| Type | Prefix | Example |
|---|---|---|
| New feature | `feature/` | `feature/http-parser` |
| Bug fix | `bugfix/` or `fix/` | `fix/apply-clang-format` |
| Refactor (no behavior change) | `refactor/` | `refactor/cleanup-poll-loop` |
| Test-only work | `test/` | `test/chunked-encoding` |
| CI/tooling/docs/infra | `chore/` | `chore/add-ci-workflow` |

Commands:

```bash
git checkout main
git pull origin main
git checkout -b feature/http-parser
```

> **Double-check spelling before pushing.** A typo in a branch name doesn't break anything functionally, but makes later cleanup commands (`git push origin --delete ...`) fail if you type the *corrected* spelling instead of the *actual* one. Not fatal — just re-run the delete with the branch's real name — but easy to avoid by checking `git branch` right after creating it.

---

## 3. Doing the work

### 3.1 Write code as usual, committing along the way ✅

Commit early and often on your branch — small, focused commits are easier to review than one giant commit at the end.

```bash
git add <files>
git commit -m "Add request-line parsing"
```

### 3.2 Tie commits to the issue ✅

Every commit message (or at minimum, the final commit / PR description) should reference the issue it's for:

```bash
git commit -m "Add request-line parsing (refs #12)"
```

Use `refs #12` for commits that are progress on the issue but don't finish it. Use one of the closing keywords (`Closes`, `Fixes`, `Resolves`) **only on the commit or PR that actually completes the issue**:

```bash
git commit -m "Complete HTTP request parsing (Closes #12)"
```

**Multiple references in one commit/PR are fine** — put each on its own line; GitHub parses them independently:
```
Scaffold tests/ directory structure (Closes #3)
refs #5
```

Referencing an already-closed issue (via `refs #N`) is safe and useful for traceability — it does not reopen it. Only `Closes`/`Fixes`/`Resolves` change an issue's state.

> **Important nuance:** the auto-close trigger fires when the commit merges into the **default branch** (`main`), not the moment you commit on your feature branch. Don't be surprised if the issue stays open until after your PR is merged.

### 3.3 Small unrelated fixes noticed while working ✅

If you notice something small and unrelated while working (e.g. a stale comment, a leftover typo) — it's fine to fix it as its **own separate commit** on the same branch, with its own message referencing the *original* issue via `refs #N`. Don't bundle it silently into an unrelated commit.

### 3.4 Scope can legitimately grow mid-issue ✅

Sometimes an issue's original description doesn't fully cover what's actually needed to satisfy its own acceptance criteria (e.g. #13 needed `tests/Makefile` filled in, which had been assumed-but-not-actually-completed under #12). When this happens: don't reopen the earlier issue — just do the necessary work under the **current** issue, note the context in the commit message (`refs #<earlier issue>`), and move on. This is normal, not a mistake to fix retroactively.

### 3.5 Adding tests ✅

New tests for a feature belong on the **same branch** as the feature they test — not a separate "tests branch." `main` should always have a fully passing test suite; your branch inherits that and adds to it.

```bash
git add tests/parsing/HttpRequestParserTests.cpp
git commit -m "Add tests for request-line parsing (refs #12)"
```

If you think of **new test cases for something already merged**, that's its own small, separate issue + branch + PR — not a reopening of old, already-merged work.

### 3.6 Run checks locally BEFORE pushing ✅

```bash
make            # build must succeed
make test       # runs the full test suite (delegates through tests/ -> tests/parsing/)
```

Fix anything that fails locally — don't rely on CI to catch it first.

> There is deliberately **no** `make style` / automated style-check target — see "Style checking" under CI & Automation Status for why this was tried and dropped.

---

## 4. Pushing your branch ✅

```bash
git push -u origin feature/http-parser
```

For subsequent pushes on the same branch:

```bash
git push
```

🔧 Once #15 (CI workflow) is set up: pushing triggers a GitHub Actions run that builds and tests automatically. Status shows on the branch/commit and later on the PR.

---

## 5. Opening a Pull Request ✅

1. Go to the **Pull requests** tab → **New pull request** (or click the "Compare & pull request" banner if GitHub shows it). This works regardless of which branch you currently have checked out locally or selected in GitHub's UI — PRs are independent of "current branch view."
2. Confirm `base: main` ← `compare: <your-branch>` — **this is the step most likely to be wrong** (both sides can default to `main`); double check before creating.
3. Title: short summary. Description: what changed, why, plus the closing keyword(s).
4. **Explicitly request your classmate as reviewer** (sidebar → **Reviewers**) — don't skip this. Assigning yourself as **Assignee** is fine and separate, but does not substitute for requesting a **Reviewer**. Note: GitHub will not let you select yourself as reviewer on your own PR — this is intentional.
5. Move the issue's Kanban card to **In Review**.

🔧 Once branch protection requires status checks (after #15 is verified working), the PR page will show required checks and block merge until they pass.

---

## 6. Review & merge ✅

1. Classmate reviews the PR — comments, requests changes, or approves.
2. If changes are requested:
   ```bash
   git add <files>
   git commit -m "Address review feedback (refs #12)"
   git push
   ```
   Updates the same PR automatically.
3. **Before merging, visually confirm you can see an actual green "Approved" review from your classmate on the PR page** — do not merge your own PR without this. (Branch protection enforces this via "Required approvals" ≥ 1 under Settings → Rulesets → your ruleset → "Require a pull request before merging" → "Show additional settings" — but check visually too, don't rely on the setting alone.)
4. Merge via **Squash and merge** (team convention — keeps `main`'s history to one commit per feature).
5. **Click "Delete branch" on the PR page immediately after merging** — easy to forget in the moment; do it before navigating away.
6. Because the merge commit/PR description contains `Closes #12`, the issue auto-closes.
7. Move the Kanban card to **Done**.

---

## 7. Sync up locally after merge ✅

```bash
git checkout main
git pull origin main
git branch -d feature/http-parser
git remote prune origin
```

If step 6.5 (delete on GitHub) was missed, the remote branch will still exist — delete it manually:
```bash
git push origin --delete feature/http-parser
```
(Use the branch's *actual* name/spelling as it exists on the remote — check with `git branch -a` first if unsure.)

---

## Quick reference — full command sequence, start to finish

```bash
# 1. Start from up-to-date main
git checkout main
git pull origin main

# 2. Branch
git checkout -b feature/http-parser

# 3. Work, committing as you go
git add <files>
git commit -m "Add request-line parsing (refs #12)"

# 4. Local checks before pushing
make && make test

# 5. Push
git push -u origin feature/http-parser

# 6. Open PR on GitHub (base: main <- compare: feature/http-parser)
#    description includes "Closes #12", REQUEST REVIEW from classmate

# 7. Wait for actual approval (check visually), then merge (squash) on GitHub,
#    click "Delete branch" immediately

# 8. Sync locally
git checkout main
git pull origin main
git branch -d feature/http-parser
git remote prune origin
```

---

## CI & Automation Status

### Test suite architecture — ✅ DONE

```
tests/
├── Makefile                # orchestrator — delegates to each tester, scales as more are added
├── TestSuite.hpp/.cpp        # shared pass/fail base class
└── parsing/
    ├── Makefile               # compiles + links this tester into run_parsing_tests
    ├── ConfigParserTests.hpp/.cpp
    ├── HttpRequestParserTests.hpp/.cpp
    └── main_parsing_tests.cpp  # aggregates both classes' results, returns correct exit code
```

**Design principle:** adding a new tester (e.g. `server/`, `response/`) later requires only a new subfolder with its own Makefile, plus one name added to `TESTERS` in `tests/Makefile` — no changes to the root Makefile or CI workflow.

**Exit code contract:** every `main_*_tests.cpp` returns `0` if all tests passed, non-zero if any failed — verified by temporarily forcing a failing `check()` and confirming propagation. This is what `make test` and CI key off of.

**Recursive Make output:** `MAKEFLAGS += --no-print-directory` is set at the top of the root Makefile, which cascades down through every nested `$(MAKE) -C ...` call automatically — suppresses the "Entering/Leaving directory" noise across the whole chain (root → tests → tests/parsing) from one place.

**Root Makefile — full chain:**
```makefile
MAKEFLAGS += --no-print-directory
# ...
test: $(NAME)
	$(MAKE) -C tests

clean:
	@rm -rf $(OBJ_DIR)
	@$(MAKE) -C tests clean

fclean: clean
	@rm -f $(NAME)
	@$(MAKE) -C tests fclean
```
`clean`/`fclean` cascading into `tests/` too means a single `make fclean` (or `make re`) from the repo root resets *everything*, not just webserv's own build artifacts.

### Style checking — ❌ ATTEMPTED, DROPPED

A `.clang-format` config was built and tested (targeting the school environment's `clang-format` v14) attempting to approximate the project's established hand-written style: tabs, Allman braces, `&name` reference style, and 42-C-norm conventions like `# define` (indented preprocessor directive) and multi-space alignment between return type and function name.

**Result:** v14 could not be configured to reliably reproduce several of these without side effects — most notably:
- `# define` inside header guards was unconditionally collapsed to `#define`, with no working v14 option found to prevent it
- Class member/access-modifier indentation (`private:`, etc.) came out mixing spaces and tabs regardless of `AccessModifierOffset` settings tried
- Multi-line stream expressions (`std::cout << ... << ...`) got re-indented using spaces, again mixing with the project's tabs
- The classic 42-style multi-space column alignment between return type and function name (`void    functionName(...)`) has no clang-format equivalent at all — it only aligns *consecutive* declarations with no blank lines/other code between them, not whole-file column alignment across separate function definitions

**Decision:** rather than force the codebase into `clang-format`'s defaults (a real, working option, just not matching the team's existing style) or keep fighting version-specific config limitations, automated style enforcement was dropped entirely. `.clang-format` was removed from the repo. Style consistency is maintained by convention and PR review instead, not tooling.

**If revisited later:** worth checking whether a newer `clang-format` version (or an alternative tool) has better support for `IndentPPDirectives` and cross-declaration alignment before trying again — this was specifically a v14 limitation, not necessarily true of all versions.

### CI workflow (`.github/workflows/ci.yml`) — 🔧 IN PROGRESS (#15)

```yaml
name: CI
on:
  pull_request:
    branches:
      - main

jobs:
  build-and-test:
    runs-on: ubuntu-24.04
    steps:
      - name: Checkout code
        uses: actions/checkout@v4
        # NOTE: this is NOT the same as `git checkout`. The CI runner starts as a
        # completely empty, fresh virtual machine every run — actions/checkout
        # does the equivalent of a fresh `git clone` + checkout of the PR's
        # commit, since there is no existing local repo on the runner to switch
        # branches within.

      - name: Set up C++ environment
        run: |
          sudo apt-get update
          sudo apt-get install -y build-essential
        # Also starting from scratch every run — the fresh VM has no compiler
        # installed by default. No clang-format install needed — style checking
        # was dropped (see above).

      - name: Build the project
        run: make

      - name: Run tests
        run: make test
```

No style-check step (dropped, see above). Kept build and test as **separate steps** so the Actions log clearly shows which stage failed, rather than one opaque combined step.

### Branch protection sequencing — why order matters

Do **not** enable "Require status checks to pass" on the `main` ruleset until after the CI workflow above has been verified working on a real PR. Enabling it before any check exists can block all merging, since there's nothing for the rule to reference as "passing." Sequence: build the workflow → verify it runs and reports correctly on a real PR → **then** enable the status-check requirement.

### Known gap fixed during setup

Early on, "Require a pull request before merging" was enabled without also setting "Required approvals" ≥ 1 (a collapsed sub-setting under "Show additional settings"). This allowed a PR to be merged by its own author with no actual review (happened once, on PR #7 — low-risk content, left as-is rather than reverted). Confirmed fixed by setting Required approvals to 1. **Always visually confirm an approval exists before merging**, in addition to relying on the ruleset.

### Status summary

- [x] `TestSuite` base class
- [x] `ConfigParserTests` class + Makefile
- [x] `HttpRequestParserTests` class
- [x] Parsing test runner + `tests/Makefile` orchestrator
- [x] `make test` wired into root Makefile (including cascading `clean`/`fclean`)
- [x] `.clang-format` config — attempted, dropped (see above)
- [ ] `.github/workflows/ci.yml` — in progress, #15
- [ ] Enable "require status checks" on `main` ruleset (after #15 is verified working)
- [ ] Server behavior tester (`tests/server/`) — placeholder issue created, not started
- [ ] Response tester (`tests/response/`) — placeholder issue created, not started