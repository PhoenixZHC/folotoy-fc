# 更新日志 / Changelog

## 最新烧录与完整包（2026-10-12）

FC 1.4 全页面 Select 返回版已写入 COM14，仅更新 0x10000 应用，保留 16 款游戏、存档和设置；写入哈希及启动身份校验通过。GameBoy 1.8 本批未烧录。全页面长按实际操作仍待用户反馈。 最新完整包和精确哈希见 [1.4 交付](docs/release-1.4.md)。

## 1.4（2026-10-12，本地测试版 / local test build）

- 游戏管理 Wi-Fi 名称改为 `folotoy-fc`，屏幕显示完整 `192.168.4.1`，修复地址截断；此追加修正尚未烧录。
- Rename the management Wi-Fi to `folotoy-fc` and show the full `192.168.4.1` address without clipping. This follow-up has not been flashed yet.
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
