# webserv — Team Workflow

This document describes the full workflow for working on this project, from picking up an issue to merging finished work into `main`. Follow it for every change, no matter how small.

**Status legend used below:**
- ✅ Works today with plain Git/GitHub — no setup needed
- 🔧 Requires setup we haven't built yet — tracked as issues #14–#18 (see "CI & Automation Status" section at the bottom)

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
   - **Acceptance criteria**: a short checklist (`- [ ]` items) describing exactly how we'll know it's done — see examples in past issues (#8–#18)
3. Assign yourself to the issue.
4. On the Kanban board, move the issue card from **Todo → In Progress**.
5. Note the issue number (e.g. `#12`) — you'll need it for your branch and commits.

---

## 2. Branch naming convention ✅

Create a branch **from an up-to-date `main`**, named to describe the work:

| Type | Prefix | Example |
|---|---|---|
| New feature | `feature/` | `feature/http-parser` |
| Bug fix | `bugfix/` | `bugfix/testcase-44` |
| Refactor (no behavior change) | `refactor/` | `refactor/cleanup-poll-loop` |
| Test-only work | `test/` | `test/chunked-encoding` |
| CI/tooling/docs | `chore/` | `chore/add-ci-workflow` |

Commands:

```bash
git checkout main
git pull origin main
git checkout -b feature/http-parser
```

> **Double-check spelling before pushing.** A typo in a branch name (e.g. `stucture` vs `structure`) doesn't break anything functionally, but makes later cleanup commands (`git push origin --delete ...`) fail if you type the *corrected* spelling instead of the *actual* one. Not fatal — just re-run the delete with the branch's real name — but easy to avoid by checking `git branch` right after creating it.

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

**Multiple references in one commit/PR are fine** — e.g. a commit that finishes one issue but also references a small unrelated fix. Put each on its own line if referencing more than one; GitHub parses them independently:
```
Scaffold tests/ directory structure (Closes #3)
refs #5
```

> **Important nuance:** the auto-close trigger fires when the commit merges into the **default branch** (`main`), not the moment you commit on your feature branch. Don't be surprised if the issue stays open until after your PR is merged.

### 3.3 Small unrelated fixes noticed while working ✅

If you notice something small and unrelated while working (e.g. a stale comment, a leftover typo) — it's fine to fix it as its **own separate commit** on the same branch, with its own message referencing the *original* issue via `refs #N` (not `Closes`, since that issue may already be closed). Don't bundle it silently into an unrelated commit.

### 3.4 Adding tests ✅ (test runner itself: 🔧 #14–#16)

New tests for a feature belong on the **same branch** as the feature they test — not a separate "tests branch." `main` should always have a fully passing test suite; your branch inherits that and adds to it.

```bash
git add tests/parsing/HttpRequestParserTests.cpp
git commit -m "Add tests for request-line parsing (refs #12)"
```

If you think of **new test cases for something already merged**, that's its own small, separate issue + branch + PR — not a reopening of old, already-merged work.

### 3.5 Run checks locally BEFORE pushing ✅ (once tooling exists: 🔧)

```bash
make            # build must succeed
make test       # 🔧 pending #14–#17
make style      # 🔧 pending #18 — likely: clang-format --dry-run --Werror ...
```

Fix anything that fails locally — don't rely on CI to catch it first.

---

## 4. Pushing your branch ✅

```bash
git push -u origin feature/http-parser
```

For subsequent pushes on the same branch:

```bash
git push
```

🔧 Once #18 (CI workflow) is set up: pushing triggers a GitHub Actions run that builds, tests, and style-checks automatically. Status shows on the branch/commit and later on the PR.

---

## 5. Opening a Pull Request ✅

1. Go to the **Pull requests** tab → **New pull request** (or click the "Compare & pull request" banner if GitHub shows it). This works regardless of which branch you currently have checked out locally or selected in GitHub's UI — PRs are independent of "current branch view."
2. Confirm `base: main` ← `compare: <your-branch>` — **this is the step most likely to be wrong** (both sides can default to `main`); double check before creating.
3. Title: short summary. Description: what changed, why, plus the closing keyword(s).
4. **Explicitly request your classmate as reviewer** (sidebar → **Reviewers**) — don't skip this. Assigning yourself as **Assignee** is fine and separate, but does not substitute for requesting a **Reviewer**.
5. Move the issue's Kanban card to **In Review**.

🔧 Once branch protection requires status checks (issue #18 → then update the ruleset), the PR page will show required checks and block merge until they pass.

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
3. **Before merging, visually confirm you can see an actual green "Approved" review from your classmate on the PR page** — do not merge your own PR without this, even if the merge button appears clickable. (Branch protection should enforce this via "Required approvals" ≥ 1 under Settings → Rulesets → your ruleset → "Require a pull request before merging" → "Show additional settings" — but don't rely on the setting alone; check visually too.)
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
make && make test && make style          # 🔧 test/style targets pending

# 5. Push
git push -u origin feature/http-parser

# 6. Open PR on GitHub (base: main ← compare: feature/http-parser)
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

Tracking issues #14–#18 build out the full CI pipeline. This section is the "how the machinery works / what's left" reference (folded in here rather than a separate doc, to keep one source of truth).

### Test suite architecture

```
tests/
├── Makefile                # orchestrator — scales as more testers are added
├── TestSuite.hpp/.cpp        # shared pass/fail base class
└── parsing/
    ├── Makefile
    ├── ConfigParserTests.hpp/.cpp
    ├── HttpRequestParserTests.hpp/.cpp
    └── main_parsing_tests.cpp
```

**Design principle:** adding a new tester (e.g. `server/`, `response/`) later requires only a new subfolder with its own small Makefile, plus one line added to `TESTERS` in `tests/Makefile` — no changes to the root Makefile or CI workflow.

**Exit code contract:** every `main_*_tests.cpp` must return `0` if all tests passed, non-zero if any failed. This is what `make test` and CI actually key off of — verify this explicitly (temporarily force a failing `check()` and confirm the exit code) before considering a tester "done."

### Root Makefile addition

```makefile
test: $(NAME)
	$(MAKE) -C tests
```

Making `test` depend on `$(NAME)` means `make test` alone always works correctly — no need to remember to run plain `make` first.

### Style checking (pending #18)

Planned approach: `.clang-format` config at repo root, approximating 42's C-norm style (indentation, brace placement). CI step:
```bash
clang-format --dry-run --Werror $(find src include -name '*.cpp' -o -name '*.hpp')
```
`--dry-run` = report only, don't modify files. `--Werror` = non-zero exit if anything doesn't match, which is what makes this usable as a CI gate.

### CI workflow (pending #18)

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
          sudo apt-get install -y build-essential clang-format
        # Also starting from scratch every run — the fresh VM has no compiler
        # or tools installed by default.

      - name: Build the project
        run: make

      - name: Run tests
        run: make test

      - name: Check code style
        run: clang-format --dry-run --Werror $(find src include -name '*.cpp' -o -name '*.hpp')
```

Kept as **separate steps** (not one combined `make && make test`) so the Actions log clearly shows *which* stage failed (build vs. test vs. style) rather than one opaque combined step.

### Branch protection sequencing — why order matters

Do **not** enable "Require status checks to pass" on the `main` ruleset until after #18's workflow has been verified working on a real PR. Enabling it before any check exists can block all merging, including legitimate ones, since there's nothing for the rule to reference as "passing." Sequence: build tests → build CI workflow (#18) → verify it runs and reports correctly on a real PR → **then** go back and enable the status-check requirement.

### Known gap fixed during setup

Early on, "Require a pull request before merging" was enabled without also setting "Required approvals" ≥ 1 (a collapsed sub-setting under "Show additional settings"). This allowed a PR to be merged by its own author with no actual review (happened once, on PR #7 — low-risk content, left as-is rather than reverted). Confirmed fixed by setting Required approvals to 1. **Always visually confirm an approval exists before merging**, in addition to relying on the ruleset.

### Remaining issues (see GitHub Issues #14–#18 for exact numbers/scope)

- [ ] `TestSuite` base class
- [ ] `ConfigParserTests` class + Makefile
- [ ] `HttpRequestParserTests` class
- [ ] Parsing test runner + `tests/Makefile` orchestrator
- [ ] `make test` wired into root Makefile
- [ ] `.clang-format` config
- [ ] `.github/workflows/ci.yml`
- [ ] Enable "require status checks" on `main` ruleset (after the above is verified working)
- [ ] Server behavior tester (`tests/server/`) — placeholder issue created, not started
- [ ] Response tester (`tests/response/`) — placeholder issue created, not started