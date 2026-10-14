# RC001 input repair and fixed controller test page (2026-10-14)

[中文](rc001-input-fix.md).

The original complete controller test layout is restored for all supported external input devices. Sticks, shoulders, triggers and X/Y are no longer hidden for the remote. Absent inputs remain zero. Mapped keyboard controls update the same page; the keyboard-only blocking message is removed. The user confirmed visible responses and correct single-key behavior.

The physical remote is **RC001, firmware 2671, VID/PID 2717:32B8**. Report ID 1 declares three 16-bit usages (6-byte payload), but sends an 8-byte boot-keyboard payload. BTstack prepends the report ID, producing 9 bytes. A captured Right report is `01 00 00 4F 00 00 00 00 00`. The previous strict length check expected 7 bytes, rejected every key and released game input.

The compatibility branch is restricted to Xiaomi identity, report ID 1, declared payload size 6, total length 9 and zero reserved byte. It decodes modifiers/reserved/six byte-sized usages, applies the requested horizontal mapping and prevents a second descriptor-based interpretation. Other reports retain descriptor validation; ordinary keyboards are isolated. Regression tests replay the physical 86-byte map and observed key formats. Constructed combinations remain distinct from device evidence.

- **25 host groups passed, 0 failed**. The physical format reproduced failure before repair and passes afterward.
- ESP-IDF 5.5.3 production entry, ESP32-C3, 8 MiB, original sixteen-game ROM partition; build, packaging and identity checks pass.
- Full flash at 0x0 on COM14 passed the write-hash check. Startup confirmed `FC_READY version=1.5 games=16` and matching ELF prefix `eac20ced2...`; no panic/abort/watchdog was observed.
- **124 physical keyboard reports** were captured after repair. All eight mapped controls and zero on release were observed; no invalid-length reports remained.
- The user confirmed correct single-key behavior but could not move and shoot simultaneously in Contra. Every captured report contains at most **one nonzero keycode**. This RC001/firmware combination currently does not provide reliable held combinations and cannot fully replace an action-game controller. Mapping cannot reliably invent missing releases. No lock, autofire or fabricated combination was added.
- Monitoring ended and **COM14 is released**. GameBoy is unchanged; nothing was committed or pushed.

Physical single-key acceptance used the **input-diagnostic variant**, with `FC_INPUT_TRACE=ON` and the other flags off. It has subsequently been replaced by the trace-disabled [menu-entry repair build](menu-entry-fix.en.md). The identities below preserve the diagnostic acceptance stage.

Archive: `deliverables/FC-1.5-rc001-report-fix-inputdiag-16games/`.

- Full image, 8,388,608 bytes: `f4962689cb1a6c06aee2df5c728ffb03f7145aad694a897f0745fc92e3f72164`.
- App, 2,541,040 bytes: `2a3bad6b9e45098fede680e17b31b3bf0cdc2ea445bd71e3bc97ada83bd89307`.
- Matching ELF: `eac20ced2ccd364acfb2ac576aeb53d149365cc7a220d9f49040453172177206`.
- ROM partition: `44d35274ace0448fc0aa523d4449d9436f419705b245d272cb5974ab174d05dd`.

The archive retains manifests, ELF/MAP, configuration, host results, flash/input logs and `device-test-result.json`. Raw logs remain local because they include nearby device information. The first layout-only diagnostic image and failure capture remain separately archived as historical evidence.
