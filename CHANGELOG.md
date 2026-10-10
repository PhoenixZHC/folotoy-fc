# 更新日志 / Changelog

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
