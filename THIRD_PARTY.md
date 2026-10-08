# 来源和本地改动

- 硬件 BSP、蓝牙输入、菜单、网页管理和存档框架：本地 `E:\code\folotoy-gameboy`，参考提交 `f221fe83fc40ec07f37f93bb19ce3d74879e8718`。原工程未修改。许可见 `licenses/FoloToy-AI-Passport.txt`。
- NES 核心：[ducalex/retro-go](https://github.com/ducalex/retro-go/tree/4ced120669750ca7228fd0414211430c1d923166/components/nofrendo)，固定提交 `4ced120669750ca7228fd0414211430c1d923166`。下载按 Git blob SHA1 校验；完整来源文件头保留。目录自带 `COPYING` 为 GPL v2，部分源文件头注明 GNU Library GPL v2；保留全部声明，不能将整包当作宽松许可。
- 本地核心修改：只编译 Mapper 0/1/2/3/4/7/23/66；移除文件系统加载及上游前端入口；检查调色板/APU 分配失败；16 kHz 每帧 266/267 样本余数累计；Mapper 23 重置寄存器；PPU 复位时初始化显存地址递增步长；APU 相位改用 Q16 定点；CPU 解释器局部开启 GCC switch 跳转表；无 Mapper latch 回调的跳过绘制路径只检查精灵 0。优化参考与验证边界见 `docs/performance.md`。
- Bluepad32：Apache-2.0，见 `licenses/Bluepad32-LICENSE.txt`。
- BTstack：见 `licenses/BTstack-LICENSE.txt` 及源码文件头；源码含非商业使用条款。此工程未完成商业再分发许可核验。
- 中文字库：Noto Sans SC Bold 700 的已生成点阵，SIL OFL 1.1，见 `licenses/NotoSansSC-OFL.txt`。
- ESP-IDF 及托管组件保留各自版权声明和许可，版本锁定在 `dependencies.lock`。
- [PassportSim](https://github.com/BlackHole1/passportsim) v1.0.1 仅作为测试工具，没有链接进固件。Windows 官方包 SHA256：`ed0e8f805125475255b59b7f45df4b5da702d925f062f6701021e6fc72e38904`。
- `tools/make_diagnostic_rom.py` 生成本项目自有诊断程序。用户提供的游戏 ROM 仅用于本地验证，不纳入默认固件或源码分发。
- 用户授权的 1.0 / 16 款完整预装包仅在本地交付，ROM 来源和哈希见 selection-full16.json；未上传或公开发布。完整包附本文件和 licenses，未因阶段收尾改变上述许可边界。
