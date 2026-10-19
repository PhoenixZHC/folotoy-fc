# Game orientation (2026-10-19)

Final acceptance: the user confirmed portrait, landscape, flipped landscape, pause/resume and saved defaults. The original sixteen-game full image is delivered; see [verified delivery](release-1.6-orientation.en.md). Pending-feedback statements below retain historical test stages and are superseded by this confirmation; endurance is not implied.

The user confirmed all original portrait/landscape tests, including pause/resume and saved defaults after reboot. A subsequent addition cycles portrait→landscape→flipped landscape→portrait. Flipped landscape rotates counterclockwise90 degrees (hold the device clockwise),180 degrees opposite ordinary landscape. All menus remain portrait. NVS retains0/1 meanings and adds2 for flipped landscape. Current-session changes do not modify the saved default.

The two affected test groups passed again;27 unaffected groups reuse valid results. Build and image checks passed. New app SHA256:e0de0250ebe4a103204d205724b4c140b62cdc48f58158e4217c98a33cd96db5; ELF:f09fd54faecc79434a99159c4d5f06a290e8df40cbb990e39aec20cc1cf13ccc. Artifacts/logs:.work/orientation-flip. The new flipped mode awaits physical acceptance. Original-build details below are historical.

[中文](game-orientation.md)

Flipped-mode build was flashed to COM13 application0x10000 only. Write hash and startup ELF identity passed; FC_READY reports1.6 and16 games. The serial port is released; new-mode physical tests remain pending.

Only game pixels rotate. Home, lists, settings, pause, messages and the launch animation remain portrait. Portrait is the default when no preference exists. The sixth pause item switches the current game's orientation on resume without changing the saved default. The fourth Settings item stores the default for subsequent game launches, including after reboot; it does not modify the current session.

Portrait retains the 240×225 rendering at (0,40). Landscape rotates the native256×240 image clockwise to240×256 at (0,32), without cropping or stretching; hold the device counterclockwise. The portrait game hint is omitted in landscape to avoid overlap. Rendering retains the existing strip/DMA worker, drains before transitions and clears on resume, without panel rotation or another framebuffer. Controller directions keep their in-game meaning.

Six pause rows use34-pixel spacing; existing menus retain their layout. Preference fc_settings/landscape accepts0/1 and defaults to portrait when missing. Save failures restore the old value and display an error.

29 host groups pass,0 fail (28-group run plus the separately executed production settings/pause/resume test). ESP-IDF5.5.3 build, image checksum, app/ELF and device partition identity pass. Only COM13 application0x10000 was flashed, leaving game/save/settings/bond partitions untouched. Startup confirms1.6 and16 games. App SHA256: df77a5a322628c1115f3dabaf86fee3a6d8144e6811f589d64f0a88cf802a30c; ELF:6b55d1d1557e8070909ab6cd243b837320ca41ec2e73892997aee227063dd898.

This is a subsequent1.6 orientation test build. The archived FC-1.6-16games full package remains unchanged and does not contain this feature. Exact artifacts/logs are local under.work/orientation-tests. Physical appearance, performance, menu transitions and persistence after reboot still need user confirmation.
