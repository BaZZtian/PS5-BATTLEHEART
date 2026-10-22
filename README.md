<p align="center">
  <img src="assets/branding/battleheart-icon-v2.png" width="160" height="160" alt="BattleHeart project icon">
</p>

<h1 align="center">BattleHeart</h1>

<p align="center">
  <strong>Sonic Adventure 2: Battle on PlayStation 5</strong><br>
  An independent native PS5 homebrew project by <a href="https://github.com/BaZZtian">BaZZtian</a>.
</p>

<p align="center">
  <a href="#status-and-availability"><img src="https://img.shields.io/badge/status-in%20development-orange" alt="Status: in development"></a>
  <img src="https://img.shields.io/badge/platform-PS5%20homebrew-003791" alt="Platform: PS5 homebrew">
  <a href="#requirements"><img src="https://img.shields.io/badge/game%20data-Own%20GameCube%20ISO-555555" alt="Game data: Own GameCube ISO"></a>
</p>

<p align="center">
  <a href="#status-and-availability">Status</a> &middot;
  <a href="#installation">Installation</a> &middot;
  <a href="#compatibility">Compatibility</a> &middot;
  <a href="#support">Support</a> &middot;
  <a href="#credits-and-licenses">Credits &amp; licenses</a>
</p>

## About

BattleHeart is a dedicated native PlayStation 5 homebrew port project for
**Sonic Adventure 2: Battle (GameCube)**.

The intended experience is a standalone PS5 application using your own game
image. The target firmware is **12.40**.

## Status and availability

> [!WARNING]
> **No playable PS5 build is available.** The published source snapshot is not an installable game.

**In development. No playable PS5 build or installable application is available
at this time.**

The existing `v0.0.1` release is an early source snapshot, not a game download.

The current source contains the native read-only media adapter, GameCube
filesystem validation and executable-boundary checks. This is groundwork for
the port. Successful component builds do not establish console gameplay.

The [Prospero Store reservation](https://github.com/darkxex/ps5-homebrew-catalog/blob/main/apps/PPSA99732.json)
uses title ID **PPSA99732**. It is a **Coming soon** placeholder with no playable
download. There is no game installation procedure to follow yet.

## Requirements

- Your own legally obtained **Sonic Adventure 2: Battle GameCube disc image**.
- A homebrew-capable **PlayStation 5**. The project's target firmware is **12.40**.

These are the intended game requirements; a console-ready application is still
in development.

## Your game data

You must provide your own legally obtained GameCube disc image. The application
is intended to read your game data locally. No ISO, original executable,
textures, soundtrack or saves are supplied or downloaded by this project.

## Installation

There is no playable application to install yet. Installation instructions will accompany a tested application release on the [Releases page](https://github.com/BaZZtian/PS5-BattleHeart/releases). The existing `v0.0.1` source snapshot and Store reservation are not playable downloads.

## Compatibility

| GameCube edition | Disc ID | PS5 gameplay |
| --- | --- | --- |
| Europe / PAL | `GSNP8P` | Not verified |
| North America / NTSC-U | `GSNE8P` | Not verified |

Both regions require real gameplay verification before a downloadable game
release. Disc identification alone does not establish gameplay compatibility
or support for every revision.

## Support

Use [GitHub Issues](https://github.com/BaZZtian/PS5-BattleHeart/issues) for project
questions and compatibility feedback. Describe the problem clearly and remove
personal information before sharing any report.

Do not upload game images, extracted game files, music, saves, keys, credentials
or proprietary SDK material.

## Credits and licenses

| Author / project | Contribution |
| --- | --- |
| [BaZZtian](https://github.com/BaZZtian) | BattleHeart maintenance and project-specific PS5 integration work. |
| [roeming and the SA2B contributors](https://github.com/roeming/SA2B) | GameCube SA2B decompilation research reference. |
| [nlopez99 and the SA2B reconstruction contributors](https://github.com/nlopez99/SA2B) | Additional GameCube reconstruction research references. |
| [ps5-payload-dev contributors](https://github.com/ps5-payload-dev/sdk) | Open PS5 homebrew SDK used for the diagnostic cross-build. |
| [LLVM contributors](https://github.com/llvm/llvm-project) | Clang/LLVM compiler toolchain. |

The SA2B projects above are research references, not an integrated game engine
in this snapshot. Their authors retain credit for their own work. BattleHeart
does not claim authorship or ownership of upstream projects or the original game.

### Licensing

The existing project-authored media tooling is distributed under the
[MIT license](LICENSE). Retain its copyright and license notice when reusing it.
Third-party components retain their own licenses and authorship; referencing an
upstream project does not mean its engine is included here.

Sonic Adventure 2: Battle and its characters are associated with SEGA and their
respective rightsholders. The project license does not grant rights to the
original game, reconstructed game code, soundtrack or character artwork.
BattleHeart is an unofficial homebrew project and is not affiliated with or
endorsed by SEGA or Sony.
