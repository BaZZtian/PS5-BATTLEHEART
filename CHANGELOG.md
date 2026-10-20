# BattleHeart changelog

## v0.0.1 — alpha source snapshot — 19 October 2026

This source snapshot documents the current development baseline. It is not a
playable PS5 release and does not publish a game package.

Included:

- read-only GameCube disc identity, FST and DOL-bounds inspection;
- local PAL/NTSC input preparation with private hashes and test templates;
- synthetic host tests, CMake/CTest integration and the release-evidence gate;
- regional compatibility templates and community guidance;
- upstream research notes and the metadata-only Prospero Store reservation for
  `BattleHeart` / `PPSA99732`.

Verified at this snapshot:

- the owned PAL revision-0 image passed local structure and DOL-bounds checks;
- the native PS5 gameplay path, graphics, audio, controller, save/load,
  Chao/Battle and multiplayer checks remain unimplemented or untested;
- no NTSC-U image is available locally, so no NTSC-U gameplay result is claimed;
- no ISO, DOL, REL, original asset, reconstructed game code or private build is
  part of the source snapshot.

The metadata reservation was merged through [PR #420](https://github.com/darkxex/ps5-homebrew-catalog/pull/420).
It does not publish a playable package. The next version must only be a bug fix
release after a real, reproducible source change; a playable game release
requires the release gate and both regional PS5 evidence sets to pass.
