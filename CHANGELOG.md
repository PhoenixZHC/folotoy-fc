# 更新日志 / Changelog

## 1.5（2026-10-14，十六游戏版已烧录 / sixteen-game build flashed）

- 用户后续授权源码同步：47 份源码、测试和文档已推送 main，提交 [00c18a3](https://github.com/PhoenixZHC/folotoy-fc/commit/00c18a3493a47184d07587ed5eb1bdde9ec0cdc9)，远端核验一致。中英文表单使用说明/更新日志已保存；游戏、完整固件和日志仅本地保留。
- Subsequently, the user authorized a source push: 47 source, test and documentation files were pushed to main at [00c18a3](https://github.com/PhoenixZHC/folotoy-fc/commit/00c18a3493a47184d07587ed5eb1bdde9ec0cdc9), with the remote hash verified. Bilingual form instructions/changelogs are included; ROMs, full firmware and device logs remain local.
- 当前菜单修复日常版完整 16 款 BIN 已交付复核，8 MiB、0x0 安装，组件/ELF/原 ROM 分区一致性通过；项目文档与 Obsidian 同步。本次未重新编译或烧录，完整安装会覆盖设备数据。详见 [完整交付](docs/release-1.5.md)。
- The current production menu-fix sixteen-game full image passed delivery revalidation (8 MiB, installation at 0x0), including component/ELF/original-ROM identity checks. Documents and Obsidian were synchronized. No rebuild or flash was performed this delivery; complete installation replaces device data. See [full delivery](docs/release-1.5.en.md).
- 最新修复主页入口：已连接显示“游戏列表”，断开显示“连接手柄”；列表入页及游戏返回建立当前持键基线，避免直接启动首个游戏。26 组主机通过、0 失败，用户反馈菜单“测试通过”。COM14 仅更新应用，保留 16 款游戏、存档、设置和配对；当前诊断开关关闭，启动身份校验通过，串口已释放。见 [菜单修复记录](docs/menu-entry-fix.md)。
- Latest repair: the first home item switches between game list and pairing with connection state. Entry and game-return held-key baselines prevent unintended game launch. 26 host groups pass, 0 fail; the user confirmed the physical menu checks. COM14 received an application-only production update preserving 16 games, saves, settings and bonds; startup identity passed and the port is released. See [menu repair](docs/menu-entry-fix.en.md).

- 后续恢复所有外接设备共用的完整手柄测试页，不按遥控器隐藏控件；普通键盘也能显示映射输入。修复 RC001 / FW2671 实际 8 字节负载与声明 6 字节不一致导致的全按键拒绝。25 组主机通过，修复诊断包已写入 COM14，八键及释放串口确认，用户确认单键正常；实测不能在《魂斗罗》中同时移动/开枪，抓包最多单个键码。见 [修复记录](docs/rc001-input-fix.md)。
- Restored one complete test layout for external devices, including mapped keyboard input. Fixed RC001/FW2671's 8-byte keyboard payload despite a 6-byte declared map. 25 host groups pass; the corrected diagnostic image is flashed on COM14, with eight inputs/releases and user-confirmed single-key behavior. Physical multi-key gameplay remains unavailable (at most one keycode per captured report). See [repair record](docs/rc001-input-fix.en.md).

- 随后按用户明确要求制作原 16 款游戏完整包，从 0x0 全新写入 COM14，覆盖原数据；写入哈希与 20 秒启动身份校验通过，确认 `FC_READY version=1.5 games=16`，串口已释放。游戏分区与旧 1.4 包逐字节一致，遥控器功能仍待实测。完整身份见 [交付记录](docs/release-1.5.md)。
- Subsequently packaged the original sixteen games and performed an explicitly authorized complete flash at 0x0 on COM14. Write hash and 20-second startup identity checks passed (`FC_READY version=1.5 games=16`); the port is released. The ROM partition is unchanged; physical remote acceptance is pending. See [release record](docs/release-1.5.en.md).

- 新增小米 BLE 遥控器横握：右→上、上→左、左→下、下→右，TV→A、三横线菜单→B、主页→Start、返回→Select；Select 长按 1 秒复用暂停/页面返回。
- 按 VID/PID `2717:32B8` 启用映射；已知广播名称仅作为待探测候选，实际连接仍校验 HID 描述符。最初的遥控器专用布局已被上述固定完整测试页替代。
- 25 组主机测试、7 项打包测试、ESP-IDF 5.5.3 构建和镜像校验通过；遥控器配对、组合键、重连和游戏延迟待实机验收。GameBoy 未修改。见 [说明](docs/xiaomi-remote.md)。
- Added horizontal Xiaomi BLE remote mapping with rotated directions, TV=A, Menu=B, Home=Start and Back=Select. Holding Back for one second uses the existing pause/back action.
- Mapping is scoped to VID/PID `2717:32B8`; names only admit discovery candidates, with the actual HID map checked after selection. The test screen shows mapped NES controls.
- 25 host groups, 7 packaging tests, ESP-IDF 5.5.3 build and image verification pass. Physical pairing, simultaneous keys, reconnect and gameplay latency remain unverified. GameBoy is unchanged. See [English notes](docs/xiaomi-remote.en.md).

## 1.4 历史烧录与完整包（2026-10-12）

FC 1.4 热点修正版（源码提交 `db1f525`）已向 COM14 的 `0x10000` 写入应用，写入哈希校验通过，保留 16 款游戏、存档和设置。20 秒启动日志确认 `FC_READY version=1.4 games=16` 和匹配 ELF，未见 panic/abort/watchdog；串口已释放。热点显示及手机连接操作仍待用户确认。

最新 16 款游戏完整包包含 Wi-Fi 修正，已校验但未整包刷写；精确身份见 [1.4 交付](docs/release-1.4.md)。

## 1.4（2026-10-12，本地测试版 / local test build）

- 游戏管理 Wi-Fi 名称改为 `folotoy-fc`，屏幕显示完整 `192.168.4.1`，修复地址截断；已烧录并通过启动检查，显示及手机连接待验收。
- Rename the management Wi-Fi to `folotoy-fc` and show the full `192.168.4.1` address without clipping. The application was flashed and startup verified; screen and phone checks remain pending.
- L1241 私有 BLE 与未知版本确认；主页手柄导航及实时测试，箭头方向、底部 Select/Start、摇杆按下变色、松开立即复原。
- 两项目长按 Select 1 秒暂停，短按及机身 OK 保留；受限 NES 2.0、RAM 声明和擦写前统一校验、热点资源清理。
- FC 24 组主机/构建通过，COM14 写入与启动确认，16 款保留。卡比音频等待/丢块及输入积压未闭环，Select 实机待反馈。
- L1241 support and confirmation for unverified firmware, controller home navigation and live test UI, one-second Select pause, bounded NES 2.0/RAM validation before erase, and hotspot lifecycle fixes.
- Build and 24 host groups pass. FC 1.4 write/startup verified on COM14 with 16 games retained; audio/input stalls and physical Select acceptance remain open. See `docs/release-1.4.md`.

## 1.3 — 2026-10-10

### 中文

1. 修复退出 Wi-Fi 游戏管理后，打开游戏提示“游戏不支持/内存不足”、需要重启才能恢复的问题。
2. 修复部分较大 ROM 在首次启动时提示“声音启动失败”的问题，调整音频初始化顺序。
3. 优化游戏画面缓冲的内存分配，降低内存碎片对游戏启动的影响。
4. 优化音频任务与缓冲区复用，减少反复进入游戏、暂停和恢复时的内存申请。
5. 优化游戏入场动画的缓冲复用，减少启动过程中的额外内存占用。

15 组主机测试、40 次实机启动链路检查、4 次 Wi-Fi 启停通过；用户确认口袋妖怪运行与暂停恢复正常。提供沿用原 16 款游戏的 1.3 完整包；实机修复验收使用标记为 1.2 的修复应用，1.3 完整包未烧录。

### English

1. Fixed game launch failures after leaving Wi-Fi game management, where “Unsupported game / insufficient memory” appeared until the device was restarted.
2. Fixed audio startup failures when launching some larger ROMs for the first time by adjusting the audio initialization order.
3. Improved framebuffer allocation to reduce launch failures caused by memory fragmentation.
4. Improved audio task and buffer reuse to reduce memory allocations when launching, pausing, and resuming games.
5. Reduced temporary memory usage during the game launch animation by reusing existing buffers.

15 host test groups, 40 on-device startup checks and 4 Wi-Fi cycles passed. The user confirmed normal Pokémon gameplay and pause/resume. Device acceptance used the repaired application labelled 1.2; the 1.3 full image retaining the original 16 games has been built and verified but has not been flashed. End-to-end HTTP upload/deletion and long-term stability were not revalidated in this round.

## 1.2 — 2026-10-10

### 中文

1. 将 Mapper 4 游戏的程序 ROM 容量上限从 512 KiB 提升至 1.5 MiB。
2. 同步调整设备上传与离线打包校验，解决符合支持条件的大容量 ROM 因旧容量限制被拒绝的问题。
3. 增加容量边界、文件截断和高位程序块切换测试，验证大容量 ROM 的解析与寻址。
4. 更新固件及游戏管理页面版本为 1.2，提供沿用原 16 款游戏的本地完整安装包。

14 组主机、7 项打包测试通过；构建、镜像校验及 COM14 启动验证通过。容量支持不代表所有游戏均兼容，实际运行效果取决于 ROM 和 Mapper 变种。真实 HTTP 上传尚未专项验收。完整包烧录地址为 0x0，会覆盖设置、游戏和存档；ROM 与完整包不在源码仓库内。

### English

1. Increased the Mapper 4 PRG ROM size limit from 512 KiB to 1.5 MiB.
2. Updated on-device upload and offline packaging validation to accept supported larger ROMs previously rejected by the size limit.
3. Added tests for size boundaries, truncated files, and high-bank switching.
4. Updated the firmware and game management page to version 1.2, with a local full installation image retaining the original 16 games.

All 14 host test groups and 7 packaging tests passed, along with build, image integrity, and on-device startup checks. Larger ROM support does not guarantee compatibility with every game or Mapper variant. End-to-end HTTP upload has not been separately verified. Flashing the full image at 0x0 overwrites settings, games, and saves. ROMs and full firmware bundles are not included in the source repository.
