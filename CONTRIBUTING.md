# Contributing

The project is at the media-validation and platform-research stage, not a playable
PS5 release. Please read [README.md](README.md) and
[NTSC-TESTING.md](community/NTSC-TESTING.md) before submitting a regional report.

## Contributions we need

- Local NTSC-U metadata reports from owners of the requested versions.
- GameCube DOL/REL runtime integration, PS5 platform adapters, and compatibility
  fixes that do not embed original program bytes or game assets.
- Synthetic regression cases for wrong regions, unknown revisions, malformed
  media, file bounds and stale build evidence.
- Later: real, build-specific PAL and NTSC-U PS5 gameplay reports.

Do not submit original game files, extracted content, console credentials, local
paths, proprietary SDKs, or links to obtain them. Only submit material you are
entitled to contribute. Keep upstream authors, licenses and notices intact.

## Code PRs

Use the PR template. Identify the relevant project and disc revisions. Explain
whether a result is synthetic, a local ISO structure check, a cross-compilation,
or a physical PS5 gameplay test. Run the CMake/CTest commands in the README.
Never replace missing regional evidence with a successful host or compiler test.

Private inputs stay outside Git. `.gitignore` is a convenience, not a content
review: do not force-add ignored inputs or disguise them with other extensions.
Review the staged files before committing. CI does not inspect your original ISO.

## Release evidence

`tools/release_gate.py <project> <candidate-folder> <evidence.json>` validates a
candidate without publishing it. The evidence must list every package file with
its SHA-256, allowed origin and license, and include both regional test records.
The checker prints the computed `build_id` even when regional evidence is missing;
use this exact value when testing that candidate. Changed files invalidate the
evidence. An ISO metadata report alone cannot unlock a release.

The permitted package origins are `own_code`, `licensed_dependency`,
`original_artwork`, `runtime_metadata`, and `license_text`. An origin label cannot
override a game-file ban. Maintainers must also review the actual content and
license obligations; the script cannot infer legal rights from a filename.

There is no workflow that creates a public release or sends private inputs to
GitHub. ProsperoStore integration will be completed against real, verified game
packages and is subject to the catalog's review process.
