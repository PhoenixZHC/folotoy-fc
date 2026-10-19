# FC 1.6 three-orientation full image (2026-10-19)

[中文](release-1.6-orientation.md) · [Instructions](release-1.6-form.en.md)

Only game pixels rotate. Modes are portrait (initial default), landscape and flipped landscape. All menus remain portrait. The sixth pause item changes the current session; the fourth Settings item saves the default across reboots. Hold the device counterclockwise for landscape and clockwise for flipped landscape.

The user confirmed the original orientation checks and then confirmed the flipped-mode checks. This covers the requested mode switching and saved defaults, not arbitrary games/controllers or endurance.

Image: `deliverables/FC-1.6-orientation-16games/FC-1.6-orientation-16games-full.bin`, **8,388,608 bytes, flash at0x0**. Original sixteen ROMs are unchanged. A complete installation replaces games, saves, settings and bonds.

- Full SHA256: `eae75e1cfba54f67a0844340aaa5b8e32de661fb39e916e32b77b128cebcb4ef`
- App SHA256: `e0de0250ebe4a103204d205724b4c140b62cdc48f58158e4217c98a33cd96db5`
- ELF SHA256: `f09fd54faecc79434a99159c4d5f06a290e8df40cbb990e39aec20cc1cf13ccc`
- ROM SHA256: `44d35274ace0448fc0aa523d4449d9436f419705b245d272cb5974ab174d05dd`

Reused the accepted ESP-IDF5.5.3 production build, all diagnostic switches off.29 host groups pass,0 fail (2 affected groups rerun,27 unaffected reused);7 packaging tests rerun and pass. Layout, five embedded components, app/ELF identity and sixteen ROMs pass verification. Components, debugging artifacts, configuration, checksums and records accompany the image.

The application matches the user-accepted build on COM13. Prior app-only flashing and startup checks passed with16 games. This delivery did not rebuild or flash the complete image. Earlier FC-1.6-16games remains an unchanged historical package without orientation support. Project documents and Obsidian were synchronized; no commit, push or public release. Existing input/audio limits and untested upload/save/endurance scenarios remain open. GameBoy is unchanged.
