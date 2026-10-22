<p align="center">
  <img src="assets/branding/battleheart-icon-v2.png" width="160" height="160" alt="BattleHeart heart emblem">
</p>

<h1 align="center">BattleHeart</h1>

<p align="center">
  A dedicated native PS5 homebrew port project for<br>
  <strong>Sonic Adventure 2: Battle - GameCube</strong>
</p>

<p align="center">
  <a href="#getting-started">Getting started</a> &bull;
  <a href="#compatibility">Compatibility</a> &bull;
  <a href="#build-from-source">Build from source</a> &bull;
  <a href="#support">Support</a> &bull;
  <a href="LICENSE">License</a>
</p>

## About

BattleHeart targets a standalone native PS5 application using the player's own
GameCube disc image. The project is dedicated to this game; the current
implementation provides its media layer. Its architecture does not use a CPU emulator,
PowerPC interpreter, JIT or a generic ISO-selection frontend.

| Project | Details |
| --- | --- |
| Source version | `v0.0.1` - alpha |
| Target platform | PlayStation 5 homebrew, firmware 12.40 |
| Original game | Sonic Adventure 2: Battle for GameCube |
| Game data | Player-provided local ISO |
| Build environment | Ubuntu 24.04 / WSL2, Clang, CMake and Ninja |
| PS5 toolchain | Open-source `ps5-payload-sdk` / Prospero |

**Current availability:** this source snapshot provides the media layer and a
command-line media diagnostic. A complete native game engine, PS5 gameplay,
graphics, audio, controller integration and an installable game package are not
available in this snapshot. Compilation and disc validation do not establish
playability.

## Getting started

You need your own GameCube ISO. BattleHeart does not include or download game
executables, textures, music or other original game content.

For this alpha, build the media diagnostic using the instructions below, then
run it on Linux with a path to your local image:

```sh
./build/host/battleheart_media_alpha "/path/to/your/game.iso"
```

The diagnostic opens the image read-only, validates its identity and file-system
structure, and reports the main executable's offset and size as JSON. It does
not extract or execute the game. A successful check exits with code `0`; a
validation failure exits with code `1`.

## Compatibility

| GameCube edition | Disc ID | Media validation | PS5 gameplay |
| --- | --- | --- | --- |
| PAL / Europe, disc 0, revision 0 | `GSNP8P` | Verified with an owned image on Linux | Not verified |
| NTSC-U / North America, disc 0 | `GSNE8P` | Implemented; synthetic inputs checked | Not verified |

Disc recognition is separate from gameplay compatibility. Other revisions and
regions have not been established as supported. PAL and NTSC-U gameplay must be
verified before a downloadable game release.

## Build from source

### Linux / WSL2

Use Ubuntu 24.04 with CMake 3.25 or newer, Ninja and Clang 18. Both C and C++
compiler packages must be available.

```sh
git clone https://github.com/BaZZtian/PS5-BATTLEHEART.git
cd PS5-BATTLEHEART
cmake -S . -B build/host -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=clang-18 \
  -DCMAKE_CXX_COMPILER=clang++-18
cmake --build build/host
```

This builds `battleheart_media_alpha` and its media libraries. No game data is
required to compile them.

### PS5 target

Install the open Homebrew `ps5-payload-sdk` separately. Set `PS5_PAYLOAD_SDK` to
its local installation directory, then configure a separate build:

```sh
cmake -S . -B build/ps5 -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$PS5_PAYLOAD_SDK/toolchain/prospero.cmake"
cmake --build build/ps5
```

The current target produces an x86-64 diagnostic ELF. Loader startup, console
execution and installable SELF packaging remain unverified. This command does
not install anything on a console. Proprietary Sony SDKs are not used.

## Repository contents

```text
assets/branding/   Project icon
runtime/          Bounded disc reader and C++ media adapters
sonic/            Game-specific disc identity checks
CMakeLists.txt    Native media library and diagnostic targets
LICENSE           Existing project license
```

Internal development records, test fixtures, research notes and private inputs
are maintained separately from this public app source tree.

## Support

Use [GitHub Issues](https://github.com/BaZZtian/PS5-BATTLEHEART/issues) for bug
reports and compatibility feedback. Include the source revision, build target,
disc ID/revision and a concise description of the problem. Review any output
before sharing it and remove personal paths or other private information.

Never upload ISOs, DOL/REL files, extracted assets, music, saves, keys, credentials
or proprietary SDK material. Keep your original image and generated game content
local.

## License and attribution

The existing project-authored media tooling is distributed under the
[MIT license](LICENSE). Retain its copyright and license notice when reusing it.
Third-party components retain their own licenses and authorship; referencing an
upstream project does not mean its engine is included here.

Sonic Adventure 2: Battle and its characters are associated with SEGA and their
respective rightsholders. The project license does not grant rights to the
original game, reconstructed game code, soundtrack or character artwork.
BattleHeart is an unofficial homebrew project and is not affiliated with or
endorsed by SEGA or Sony.
