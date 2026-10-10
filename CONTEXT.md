# 当前状态（2026-10-10）

- 最新交付已升级为 **1.3**，固件与网页同步；正式构建、镜像布局、读回、版本、ELF 身份和诊断开关校验通过，打包测试 7 项通过。完整包 `deliverables/FC-1.3-16games/FC-1.3-16games-full.bin`，8 MiB，SHA256 `6cda5794ac4ddc697ecab557963f3bcb27d2f0688f8407c3e1d6227668a71cca`；原 16 款 ROM 逐字节不变，历史包保留。写入地址 0x0，会覆盖设置/游戏/存档。**新完整包未烧录；源码纳入本批 GitHub 提交**，COM14 仍是下述已验收的 1.2 标记修复应用。见 `docs/release-1.3.md`。

- Wi-Fi 管理后启动故障修复：画面按 1088 字节分块、固定音频队列/任务栈、动画复用缓冲并释放旧 GB 比较缓存；补充在核心分配前准备 I2S/codec，解决首次直接启动大 ROM 的 DMA 分配失败。15 组主机测试有效；COM14 冷启动先开关 Wi-Fi 再大 ROM 优先的 40 次核心/动画/音频/LCD 检查和 4 次 Wi-Fi 启停通过。正常候选应用已刷（SHA256 `58c2dde4…`，ELF `933b0fa6…`），自测关闭，仅更新应用分区。用户已确认口袋妖怪运行及暂停恢复正常，本次启动故障修复验收通过；COM14 监听已停止。网页上传/删除与长期运行仍未重新验收；1.3 完整包见上，新完整包未烧录；源码纳入本批 GitHub 提交；见 `docs/wifi-memory-regression.md`。

- 保留 Mapper 4 容量上限、BLE 输入、红白菜单、240×225 彩色显示和电池 SRAM 存档；无即时存档。正式构建关闭 FC_SIM_TEST / FC_INPUT_TRACE / FC_WIFI_MEMORY_TEST。
- 未完整验收：HTTP 上传/删除全流程、存档/断电恢复、全部 ROM/外设及长期运行。历史低帧率和音频丢块限制保留。
- 项目文档与 Obsidian 已同步 1.3；历史版本记录保留。当前交付见 docs/release-1.3.md，实机验收见 docs/wifi-memory-regression.md。用户随后授权将本批源码、测试和文档提交至现有 GitHub main；完整包、ROM、存档及设备日志仅本地保留。
