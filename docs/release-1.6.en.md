# FC 1.6 full image with sixteen games (2026-10-19)

Historical controller-fix package. The latest accepted package is the [three-orientation image](release-1.6-orientation.en.md); the earlier image below has no orientation feature. Device and not-flashed statements below are historical.

[中文](release-1.6.md) · [Instructions and changelog](release-1.6-form.en.md)

Image: `deliverables/FC-1.6-16games/FC-1.6-16games-full.bin`, **8,388,608 bytes (8 MiB), flash at 0x0**. The original sixteen-game partition is byte-for-byte unchanged. Complete installation replaces games, saves, settings and Bluetooth bonds.

- Full SHA256: `06e0feec0eaff7fb96def88aac290a584b80c1744c52b6a22a0ac0a122e0ede3`
- App SHA256: `5a98eb084dfbb1dd49de1d7d62577117c4b8797cd2582884685ad6c951cbf36e`
- Matching ELF SHA256: `140dda659569a51b423c3c5db39c1c79e4ccf7c9a39def5290a4735c4e05bd1e`
- ROM partition SHA256: `44d35274ace0448fc0aa523d4449d9436f419705b245d272cb5974ab174d05dd`

BTstack now preserves the pending CCC write when discovery replies are paginated and returns an explicit error if CCC is absent at the end. This resolves the Q36 Android-mode initialization stall without extending timeouts or forcing a name-based profile. Version and management-page labels are 1.6; all four diagnostic/test switches are OFF.

The user confirmed Q36 D-mode connection, buttons and power-cycle reconnect, and both Android and Xbox modes on an IINE Pocket Controller Mini and an IINE Pocket Controller Pro (described by the user as the officially supported model). Exact hardware/firmware revisions for those two were not supplied. Q36 X mode uses Classic Bluetooth on the tested unit and is unsupported by ESP32-C3. Other Xbox or Android modes can use BLE; mode names alone do not establish compatibility.

Serial logs show ready on Xbox BLE, generic Android and IINE FW27.02 paths, but also connection retries, one unknown/truncated report and IINE timeout disconnects followed by reconnection. Whether disconnects were intentional power-off tests was not independently established. No long-term stability claim is made.

Build: ESP-IDF 5.5.3, original layout and verified sdkconfig.defaults values. Host checks: 27 passed, 0 failed, reused from the same functional code. Packaging checks: 7 passed, 0 failed, rerun. Layout, five component hashes, 1.6 app/ELF identity, disabled diagnostics and all sixteen ROMs were verified. Matching components, ELF/MAP, configuration and manifests are archived alongside the image.

Physical acceptance applies to the repaired build still labeled 1.5 on COM13 (app SHA256 `974aac7f4419a19e36ed5410af75658ef34ba99a162a25d10ffd8400c80c6029`). **The newly built 1.6 image was not flashed**; only version labels changed after that acceptance. The serial port is released. No commit, push or public release was performed. GameBoy was not changed. Historical audio/input issues, remote multi-key limits and untested durability/upload/save scenarios remain open.
