# BattleHeart

BattleHeart is an early native PS5 homebrew source project for the **Sonic
Adventure 2: Battle (GameCube)** compatibility target. It uses each player's
own GameCube ISO. Target firmware: **12.40**.

**Source version: `v0.0.1` (alpha source snapshot).** This is a tooling and
documentation release, not a playable PS5 game package.

**There is no playable PS5 port or downloadable game release here.**
The implemented code is a read-only GameCube media layer, regional input
preparation and a release-evidence checker. Synthetic tests and compilation are
not evidence of gameplay on a console.

The Prospero Store placeholder uses title ID `PPSA99732`. The original
reservation was merged through [catalog pull request #420](https://github.com/darkxex/ps5-homebrew-catalog/pull/420).
Renaming its display name to **BattleHeart** is requested in
[pull request #484](https://github.com/darkxex/ps5-homebrew-catalog/pull/484), pending maintainer approval.
It is a metadata-only listing and contains no download, game code, assets or
ISO.

## Current scope

The Xbox 360 reference is not a drop-in GameCube runtime. Xbox XEX programs, ReXGlue/Xbox services and Xenos graphics must not be assumed compatible with the GC DOL/REL programs and GX graphics. A GC-only Sonic runtime is not implemented. The Sonic media adapter currently checks identity and DOL bounds only.

| Region | Expected disc ID | Gameplay on PS5 |
| --- | --- | --- |
| NTSC-U | `GSNE8P` | Not tested |
| PAL | `GSNP8P` | Not tested |

Disc revisions are recorded separately. Recognizing a header does not establish
that a revision is compatible. Both regional gameplay reports are required for
the exact candidate build before a game release. Sonic also requires Chao/Battle and multiplayer checks.

## Help wanted

We need owners of the **NTSC-U** version to run local media checks and share
reviewed metadata, and developers to contribute compatibility work through PRs.
See [NTSC testing](community/NTSC-TESTING.md) and [Contributing](CONTRIBUTING.md),
or open a [regional compatibility report](https://github.com/BaZZtian/PS5-BattleHeart/issues/new?template=regional-compatibility.yml).

**Do not upload or send ISOs, original executables, REL modules, extracted game
content, shaders, proprietary SDKs or download links, including in private messages.**
Users always provide their own image locally. Sources and future release packages
must contain no original game files.

## Prepare an owned ISO with one command

Windows, from this repository's folder:

```powershell
.\prepare-ntsc.ps1 -Iso "D:\Own Games\game-ntsc.iso"
```

Add `-Region PAL` for PAL. Optional `-ExpectedSha256 <checksum>` rejects an image
that differs from a known checksum. This uses Python 3.10+, WSL Ubuntu-24.04,
CMake 3.25+, Ninja and Clang/GCC; it does not install software.

Linux:

```sh
python3 tools/prepare_region.py --iso "/media/own/game-ntsc.iso"
```

The command builds the current inspection tool, runs synthetic tests, verifies
the disc identity/region/number, FST and DOL bounds, and computes SHA-256/SHA-1.
It does not modify, copy, upload or execute the ISO. It prepares inputs and test
forms; it does not build a playable game. Native runtime/platform work is pending.

Outputs are Git-ignored under `private-inputs/sonic-adventure-2-battle/<region>/<sha256>/`:

| File | Purpose | Public sharing |
| --- | --- | --- |
| `input.json` | Private local ISO path and registration | Never |
| `metadata.json` | Disc identity, revision, hashes, size and structure result | After review |
| `ps5-test-template.json` | Empty form for later real PS5 testing | Only report actual results |
| `../current.json` | Local pointer to the latest registration | Keep local |

## Build and test the tools

```sh
cmake -S . -B build/host -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang
cmake --build build/host
ctest --test-dir build/host --output-on-failure
```

CI uses synthetic fixtures only. It never uploads original inputs or publishes
game packages. See [STATUS.md](STATUS.md) and [CHANGELOG.md](CHANGELOG.md) for
the current release state and limitations.

## References and licensing

- [sa2-nx](https://github.com/ChanseyIsTheBest/sa2-nx) (Xbox reference), revision `26a4213fd01414522927779c520d406c57ce0014`.
- [SA2B](https://github.com/roeming/SA2B) (GameCube reference), revision `874a480379260dce01e8988d6ab13c0deddcee4c`.

Upstream game sources are not included in this initial tooling repository.
The [MIT license](LICENSE) covers the new code here. It does not cover original
game files or grant rights to upstream projects; their own licenses, authors
and notices must be respected when integrating components.

This is an unofficial project and is not affiliated with the original developers
or publishers.
