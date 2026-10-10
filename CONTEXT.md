# 当前状态（2026-10-10）

- 当前本地交付 FC模拟器 1.2：Mapper 4 PRG 上限 1.5 MiB，CHR 上限仍 256 KiB；固件/网页版本同步。14 组主机、7 项打包测试通过，1.2 正式构建与镜像校验通过。
- 完整包 `deliverables/FC-1.2-16games/FC-1.2-16games-full.bin`，8 MiB，从 0x0 写入覆盖设置/游戏/存档；SHA256 `63b859844ddde634035520b0880d2c4f9fea25c1b9b5f4ccce18880eb5dd2caa`。沿用原 16 款 ROM，数据区剩余 4 KiB、16 槽已满，独立存档 960 KiB。完整包仅本地交付、历史包保留；源码与文档维护于 GitHub `PhoenixZHC/folotoy-fc`，ROM/固件/存档/日志不纳入仓库。
- COM14 已刷入正式 1.2 原 16 款完整包，写入校验与 25 秒启动验证通过，ELF de13f155d…；串口已释放。前一轮两款测试中皮卡丘原启动报错未复现，翡翠版按用户复测结论记录为 ROM 问题。
- 保留 BLE 输入修复、红白菜单/入场动画、彩色竖屏 240×225、核心/显示/音频优化。仅电池 SRAM 存档，无即时存档；正式构建关闭 FC_SIM_TEST / FC_INPUT_TRACE。原 GameBoy 工程未改动。
- 尚未完整验收：真实 Wi-Fi 上传、存档/断电恢复、全部外设与 BLE 重连矩阵、全部游戏与长期稳定性。历史部分场景显示低于 15 fps、少量音频丢块，不因本次收尾而消除。
- 当前交付/设备/实测分别见 `docs/release-1.2.md`、`docs/device-1.2.md`、`docs/mapper4-capacity-test.md`；历史记录保留，适用最新结论以当前文档为准。
