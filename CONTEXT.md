# 当前状态（2026-10-12）

- FC 1.4 全页面 Select 返回版已写入 COM14，仅更新 0x10000 应用，保留 16 款游戏、存档和设置；写入哈希及启动身份校验通过。GameBoy 1.8 本批未烧录。全页面长按实际操作仍待用户反馈。
- 应用 `08ea366061b22c63b33e242ac90548cd7ac5f5f915c258d4303e305e4dfb6622`，ELF `2bb636c5aa3fc1bd278a1eea95493cbdb9c699cecb77d012b728f0279d0c3d50`；启动 `FC_READY version=1.4 games=16`，串口记录 `build/com14-select-return-startup.log`。
- 最新完整包 `deliverables/FC-1.4-select-return-16games/FC-1.4-select-return-16games-full.bin`，SHA256 `6e40197dfcc0e761974b2b81f26a7078ea1c64536a9ab5b92459b639807459f1`，8 MiB/16 款；完整刷写覆盖数据。详情 `docs/release-1.4.md`。
- 两项目已同步 L1241、主页导航和测试 UI、全页面 Select 1 秒返回。FC 24 组、GameBoy 32 组主机测试通过，构建及镜像通过。GameBoy 1.8 未实机验证。
- 本次 FC 源码追加游戏管理热点 `folotoy-fc`，屏幕显示完整 `192.168.4.1`。该修正尚未烧录，以上 COM14 身份与预装包均为修正前版本。
- FC NES 2.0 诊断、网页和断电存档待验收；卡比 155 ms 音频等待、3 次丢块/2 个输入积压丢弃未定位，不因菜单修复消除。
