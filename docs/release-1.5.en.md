# FC 1.5 full image delivery with sixteen games (2026-10-14)

Historical release. See [FC1.6](release-1.6.en.md) for the latest package and test scope. Current below refers to the recorded historical date.

## Current production delivery

Image: `deliverables/FC-1.5-menu-entry-fix-16games/FC-1.5-menu-entry-fix-16games-full.bin`, **8,388,608 bytes (8 MiB), complete installation at 0x0**.

- Full SHA256: `c155fb180c0ccf07dcbd48c1f0b5ac2ab328fa1a7fb1232c094f9fa5f71fd44b`.
- App SHA256: `df35db76538a7d0273525056000c3346c4d23e02f272ca540b2d8d572e0f64a0`.
- Matching ELF: `1bb540ff885b0d6606d096d2948d90df428f7deccb65522d9519f96442b19ba2`.
- Original sixteen-game partition: `44d35274ace0448fc0aa523d4449d9436f419705b245d272cb5974ab174d05dd`.

Includes horizontal mapping, RC001 report compatibility, the shared complete controller-test screen, connection-aware home labels and held-key protection on list entry. All diagnostics are off. The original sixteen games are unchanged; see the [game list](games-full16.md).

Delivery revalidation checked full size, five embedded component hashes/bounds, partitions, app version/ELF, diagnostic flags, all sixteen entries/ROM formats/hashes, and byte identity with the original ROM partition. The archive retains the manifest, `delivery-verification.json`, `SHA256SUMS.txt`, matching ELF/MAP, configuration, components, licenses, documentation and previous acceptance evidence. This delivery reused the existing production build, **26 host groups passed/0 failed** and **7 packaging tests passed/0 failed**; no rebuild or flash was performed.

COM14 previously received only this app at 0x10000, preserving games, saves, settings and bonds. Startup version/16-game/ELF checks and user single-key/menu acceptance passed; serial is released. The full image has not been flashed as a whole this round. **Writing the complete image at 0x0 replaces games, saves, settings and bonds.** RC001/FW2671 still reports at most one captured keycode and cannot move and shoot simultaneously. See [menu acceptance](menu-entry-fix.en.md) and [RC001 evidence](rc001-input-fix.en.md). Project documents and the Obsidian knowledge base were synchronized; nothing was committed, pushed or publicly released.

## Initial complete-flash history

Subsequent source synchronization: the user authorized committing and pushing 47 source, test and documentation files, including the [English form text](release-1.5-form.en.md)/[中文](release-1.5-form.md), to main at [00c18a3](https://github.com/PhoenixZHC/folotoy-fc/commit/00c18a3493a47184d07587ed5eb1bdde9ec0cdc9). Local and remote hashes matched. Earlier “not committed/pushed” statements retain their original delivery timing. No full BIN, ROMs, ELF/MAP, saves or device logs were uploaded; no rebuild, flash or firmware Release was performed.

The following preserves the initial image/startup evidence. Those images predate the subsequent RC001 and menu-entry fixes and are not the current delivery above.

[中文](release-1.5.md) · [Remote mapping](xiaomi-remote.en.md).

At the user's explicit request, the FC 1.5 horizontal-remote application and the original sixteen-game partition were packaged and flashed to **COM14 from 0x0**. The complete installation overwrote games, saves, settings and bonding. No commit, push or public upload was performed.

- Image: `deliverables/FC-1.5-xiaomi-16games/FC-1.5-xiaomi-16games-full.bin`, **8,388,608 bytes**.
- Full SHA256: `d6e13318d4632c2d47fe7fd7833c6f8215a8f0bb4c0ff34d810f49190afb73fc`.
- App SHA256: `e2c71ebbed396e47df31f5ecf70c1f42e9a22b7e68e7e71df113f3ef5992d13a`.
- Matching ELF SHA256: `33d79967247a3a1e95d556e76ab529287f1b2e3d26cf378f14b86d3120e13723`.
- ROM partition SHA256: `44d35274ace0448fc0aa523d4449d9436f419705b245d272cb5974ab174d05dd`.

The original ROM partition was extracted from the verified FC 1.4 Wi-Fi sixteen-game image after checking its full hash and partition table. All sixteen directory entries and ROM hashes were checked. The new ROM partition is byte-identical to the source. The already-verified FC 1.5 app/ELF were reused without rebuilding; the new merged image passed layout, component-hash, app/ELF and file readback checks. The archive includes images, manifests, hashes, ELF/MAP, sdkconfig, test results and local flash/startup logs.

- Build: reused ESP-IDF 5.5.3 production build for ESP32-C3, 8 MiB, existing partitions, diagnostics off.
- Tests: reused **25 host groups passed, 0 failed** and **7 packaging tests passed, 0 failed**; additionally checked the sixteen-game container and ROM partition identity.
- Device: COM14 identified as ESP32-C3 with 8 MiB Flash. esptool 4.12.0 wrote the full image at 0x0, returned 0 and verified the written data hash. No separate whole-chip erase command was used.
- Startup: after reset, a **20-second** window confirmed project `FoloToy-FC`, version **1.5**, matching ELF prefix `33d799672...` and **`FC_READY version=1.5 games=16`**. No panic, abort or watchdog was observed. BLE initialized and scanning started.
- Serial monitoring ended; **COM14 is released**.

Physical remote pairing, all mapped keys/releases, simultaneous direction+A/B, reconnect and gameplay latency remain pending. Use the controller test page before playing. Flash/startup verification does not establish visual/audio quality or full game acceptance. Existing Kirby audio/input stalls remain unresolved.
