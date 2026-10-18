# OpenSA2B PS5 development status

Project: **OpenSA2B PS5**. Compatibility target: **Sonic Adventure 2: Battle
(GameCube)**. The public source repository is
[BaZZtian/OpenSA2B-PS5](https://github.com/BaZZtian/OpenSA2B-PS5).

## Current source release

`v0.0.1` is the published alpha source snapshot described in
[CHANGELOG.md](CHANGELOG.md) and [the GitHub release](https://github.com/BaZZtian/OpenSA2B-PS5/releases/tag/v0.0.1).
It records the inspection tools, tests, regional preparation, research and
publication metadata. It does not claim to be a playable game release.

- Read-only media parser and synthetic bounds/region tests are implemented.
- Private local ISO registration, hashes and test-report templates are implemented.
- Community report and PR templates are included.
- The common media core was cross-compiled as a PS5 static library during initial
  development; this is not an installable game or a hardware gameplay test.
- The owned PAL revision-0 image has passed real ISO structure and DOL-bounds
  checks; its SHA-256/SHA-1 are stored only in the private local registration.
- No NTSC-U image or physical PS5 gameplay test has been completed.
- No playable game release or console deployment exists for this project.
- A metadata-only Prospero Store reservation for **OpenSA2B PS5** (`PPSA99732`)
  is open as [catalog PR #420](https://github.com/darkxex/ps5-homebrew-catalog/pull/420)
  and awaits maintainer review; it is not a live listing.

## Runtime work

The Xbox 360 reference is not a drop-in GameCube runtime. Xbox XEX programs, ReXGlue/Xbox services and Xenos graphics must not be assumed compatible with the GC DOL/REL programs and GX graphics. A GC-only Sonic runtime is not implemented. The Sonic media adapter currently checks identity and DOL bounds only.

## Release and store requirements

PAL (`GSNP8P`) and NTSC-U (`GSNE8P`)
must both pass real gameplay testing on firmware **12.40** using the same candidate
build. Test records require the build ID, exact disc SHA-256 and revision, date,
evidence reference, and boot/gameplay/audio/controller/save-load checks. Sonic also requires Chao/Battle and multiplayer checks.

Use the release checker before considering a release:

```sh
python3 tools/release_gate.py sonic-adventure-2-battle <candidate-folder> <evidence.json>
```

It checks hashes, provenance declarations, required files, version metadata and
regional evidence. It cannot verify rights to mislabeled content or independently
prove that a human ran a console test; maintainer review remains required.
The current workflow validates source and synthetic fixtures only. It does not
publish a game release. Community source contributions can proceed while game
releases wait for the required evidence.

ProsperoStore integration is planned for verified native packages, with GitHub
as the version source. Catalog submission and updates remain subject to catalog
maintainer review. Immediate store approval is not guaranteed.
