# Connected entry and game-list held-key repair (2026-10-14)

[中文](menu-entry-fix.md).

When disconnected, the first home item displays “连接手柄” and opens pairing. When connected, it displays “游戏列表” and opens only the game list. Disconnection restores the pairing label; disconnecting from the list returns home, including an empty list.

Previously the label was static and the pairing loop immediately returned when already connected. The game list initialized its A baseline to false, so the held A used to confirm the home item was interpreted as a new game-start action. The fix uses the existing menu edge state machine with the actual held-state baseline, clears old board events on entry and resets the baseline after returning from a game. A release and new press are required to start a game. Existing long OK/Select back behavior remains.

**26 host groups passed, 0 failed**, exercising actual home/list functions and routing, entry holds, release/repress, return-from-game holds, directions and disconnect. A missing stdio include in the new test harness was corrected and that test rerun; the other 25 unchanged successful checks were reused.

ESP-IDF 5.5.3 production build and image/app/ELF checks passed, with all diagnostic flags off. The shared full controller-test layout and verified RC001 single-key fix are retained; physical multi-key limitations remain.

Only the application was written to **COM14 at 0x10000**, with its write hash verified. ROMs, saves, NVS/settings and bonds were not written. The separately archived sixteen-game full image was not flashed as a complete image this round.

Archive: `deliverables/FC-1.5-menu-entry-fix-16games/`.

- Full SHA256: `c155fb180c0ccf07dcbd48c1f0b5ac2ab328fa1a7fb1232c094f9fa5f71fd44b` (8,388,608 bytes).
- App SHA256: `df35db76538a7d0273525056000c3346c4d23e02f272ca540b2d8d572e0f64a0` (2,540,112 bytes).
- Matching ELF: `1bb540ff885b0d6606d096d2948d90df428f7deccb65522d9519f96442b19ba2`.
- Original ROM partition: `44d35274ace0448fc0aa523d4449d9436f419705b245d272cb5974ab174d05dd`.

The user replied “测试通过” (test passed) to the menu checks, confirming the physical menu flow. Serial startup matched `FC_READY version=1.5 games=16` and the ELF, with no panic/abort/watchdog observed. Monitoring ended and COM14 was released. This does not establish acceptance of all games or performance. Nothing was committed or pushed.

The same production sixteen-game full image was subsequently delivered and revalidated without another flash. See [full delivery](release-1.5.en.md) for usage and component/ROM checks.
