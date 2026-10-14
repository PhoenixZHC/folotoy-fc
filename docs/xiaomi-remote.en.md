# Horizontal Xiaomi BLE remote support (FC 1.5)

2026-10-14. [中文](xiaomi-remote.md).

This change implements the requested mapping in FC only. The GameBoy project is unchanged. NES and GB/GBC controls have no shoulder buttons.

| Physical remote key | Keyboard/Keypad usage | Game control |
|---|---|---|
| Right | `0x4F` | Up |
| Up | `0x52` | Left |
| Left | `0x50` | Down |
| Down | `0x51` | Right |
| TV | `0x35` / `0xC0` | A |
| Three-line Menu / Settings | `0x65` / `0x5D` | B |
| Home | `0x4A` / `0x24` | Start |
| Back | `0xF1` | Select |

Holding Back for one second uses the existing pause/page-back action. Release it before another hold after entering or resuming a page. OK, power, voice and volume keys have no game action.

Mapping is limited to keyboard-class HID devices with VID/PID `2717:32B8`. Complete names `MI RC` and `Xiaomi RC` can admit discovery candidates without a HID UUID or with Generic Remote Control appearance. A name does not activate mapping: user selection and actual HID descriptor checks still apply. All external inputs now use the original complete controller-test layout; missing controls remain zero. Mapped keyboards also use that page. The user's RC001/FW2671 single keys/releases are verified; combinations are unavailable. See [repair record](rc001-input-fix.en.md).

The existing Bluepad32/BTstack descriptor parser decodes reports, including the tested 16-bit usage array and standard keyboard layouts. Back is translated before standard keyboard filtering. Full keyboard snapshots replace previous keys; independent consumer reports preserve held keys. Invalid report lengths release game input and log an error. No ATVV voice implementation is included.

- Build: ESP-IDF 5.5.3, ESP32-C3, 8 MiB, existing partitions, production entry with diagnostic flags off; build, application/ELF matching and merged-image checks pass.
- Host tests: currently **26 passed, 0 failed**, including captured RC001 descriptor/report replay and menu label/routing/held-entry regression. Synthetic combinations verify parser capability, not physical simultaneous reporting. The new menu harness was rerun after fixing its missing header, reusing the other 25 valid results; see [menu record](menu-entry-fix.en.md).
- Packaging tests: **7 passed, 0 failed**.
- Device flash/startup: the initial sixteen-game variant was flashed from 0x0 with explicit authorization, followed by capture-based RC001 repair. The latest production menu repair updated only the COM14 application, preserving games, saves, settings and bonds. Write hash and version 1.5/16-game/ELF checks passed; no panic/abort/watchdog was observed, and the port is released. The user confirmed single-key and menu behavior; physical combinations remain unavailable. Current identities are in the [menu delivery](menu-entry-fix.en.md); the [initial release](release-1.5.en.md) is historical.

The user confirmed correct single-key behavior but cannot move and shoot simultaneously in Contra. All 124 captured reports contain at most one nonzero keycode. This RC001/FW2671 is not a full replacement for an action-game controller requiring combinations. No fabricated held combinations or autofire were introduced; other models/firmware still need separate acceptance.

The initial empty-game archive is `deliverables/FC-1.5-xiaomi-remote/`, containing the app, matching ELF/MAP, sdkconfig, bootloader, partition table, flash_args, empty full image, manifest and test results. Exact identities are in the [Chinese record](xiaomi-remote.md). The 2,540,144-byte application is for **0x10000**, never 0x0. The subsequently generated and flashed sixteen-game variant is in `deliverables/FC-1.5-xiaomi-16games/`, full SHA256 `d6e13318d4632c2d47fe7fd7833c6f8215a8f0bb4c0ff34d810f49190afb73fc`; it uses the same app/ELF. Full installation at 0x0 overwrites games, saves, settings and bonding. The historical FC 1.4 archive remains separate. Nothing was committed or pushed.

Sources consulted on 2026-10-14: [RC003 protocol and key codes](https://github.com/ncmro7/MI-RC003-ESP32-Bridge), [MiVibe device identity](https://github.com/wyuzhi/MiVibe-Remote), [SayAll RC001/RC003 scope](https://github.com/GetSayAll/remote-mic-app-windows). Only protocol/identity data informed an independent mapping implementation; external PC/S3 results do not establish C3 device acceptance.
