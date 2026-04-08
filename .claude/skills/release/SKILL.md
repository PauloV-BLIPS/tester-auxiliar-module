---
name: release
description: >
  Automate semantic version releases: bump VERSION, graduate CHANGELOG.md entries
  from [Unreleased] to a dated section, commit, and tag. Invoke as /release,
  /release patch, /release minor, /release major, or /release 1.2.3.
  Use this skill whenever the user mentions releasing, bumping version, cutting a
  release, tagging a version, or preparing a new version — even if they don't
  explicitly say "/release".
---

# Release Skill

Automates the full release workflow: version bump → changelog update → commit → tag.

## Usage

```
/release              → patch bump (0.1.0 → 0.1.1)
/release patch        → patch bump
/release minor        → minor bump (0.1.1 → 0.2.0)
/release major        → major bump (0.2.0 → 1.0.0)
/release 2.5.0        → set explicit version
```

## Workflow

Follow these steps in order. Do NOT skip any step.

### Step 1 — Parse the argument

The user passes an optional argument after `/release`. Parse it:

- No argument or `patch` → bump the patch number
- `minor` → bump the minor number, reset patch to 0
- `major` → bump the major number, reset minor and patch to 0
- A string matching `N.N.N` (e.g. `2.5.0`) → use that exact version

If the argument doesn't match any of these, tell the user the valid options and stop.

### Step 2 — Read current version

Read the `VERSION` file in the project root. It contains a single line like `0.1.0`.
Parse it into major, minor, patch integers.

### Step 3 — Calculate new version

Apply the bump rule from Step 1 to the current version. The new version must be
strictly greater than the current version. If using an explicit version, validate
this — if the explicit version is not greater, warn the user and stop.

### Step 4 — Check CHANGELOG.md

Read `CHANGELOG.md` and look for an `## [Unreleased]` section. Extract everything
between `## [Unreleased]` and the next `## [` heading (or end of file if there's
no next heading).

- If there IS content under [Unreleased] (non-empty, not just blank lines): proceed.
- If there is NO content (empty or only whitespace): warn the user that there are
  no documented changes under [Unreleased]. Ask if they want to proceed anyway.
  If they say no, stop. If they say yes, continue (the release section will be empty).
- If there is no `## [Unreleased]` section at all: add one and warn the user that
  no changes were documented.

### Step 5 — Update CHANGELOG.md

Using the Edit tool:

1. Replace `## [Unreleased]` with:
   ```
   ## [Unreleased]

   ## [X.Y.Z] - YYYY-MM-DD
   ```
   Where `X.Y.Z` is the new version and `YYYY-MM-DD` is today's date.

   The content that was under [Unreleased] should now appear under the new version
   heading. The [Unreleased] section should be left empty (ready for future changes).

### Step 6 — Update VERSION

Write the new version string (just `X.Y.Z` with a trailing newline) to the `VERSION` file.

### Step 7 — Commit

Stage `CHANGELOG.md` and `VERSION`, then create a commit:

```
release: vX.Y.Z
```

Use the standard Co-Authored-By trailer.

### Step 8 — Tag

Create an annotated git tag:

```bash
git tag -a vX.Y.Z -m "vX.Y.Z"
```

### Step 9 — Summary

Show the user a summary:

```
Release v{NEW} created successfully.

  {OLD} → {NEW}
  Commit: {short hash}
  Tag:    v{NEW}

Next steps:
  git push origin master --tags    ← push commit and tag to remote
```

## Important notes

- The CHANGELOG uses Portuguese section headers: `### Adicionado`, `### Alterado`,
  `### Corrigido`, `### Removido`, `### Depreciado`, `### Segurança`.
  Do not translate or rename these headers.
- Never push automatically — the user decides when to push.
- If the git working tree has uncommitted changes (other than the ones this skill
  makes), warn the user before proceeding — they may want to commit or stash first.
