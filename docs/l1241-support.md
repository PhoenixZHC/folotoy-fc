# 良值 L1241 支持与验证（2026-10-12）

## 全页面 Select 返回（2026-10-12）

所有原有长按 OK 返回/退出操作均支持长按 Select 1 秒，机身 OK 保留：设置（编辑时先结束编辑）、手柄测试、游戏管理、游戏列表/空列表、暂停菜单（保存成功后退出游戏）。配对取消/重试也复用该事件，但未连接或确认前没有可用手柄输入时仍需机身按键。进入页面及从子页面返回时重新建立持键基线，必须释放后再次长按，避免连续跨页退出。测试页 Select 短按用于观察，持续 1 秒返回。 FC 1.4 全页面 Select 返回版已写入 COM14，仅更新 0x10000 应用，保留 16 款游戏、存档和设置；写入哈希及启动身份校验通过。GameBoy 1.8 本批未烧录。全页面长按实际操作仍待用户反馈。


## 当前状态（2026-10-12）

FC 当前为已烧录的 1.4 Wi-Fi 修正版，保留 16 款游戏；GameBoy 当前源码 1.8，未烧录。L1241 请使用 PC 无线模式。最新身份见 [1.4 记录](release-1.4.md)。以下旧产物与未烧录描述属于历史批次。

源码新增 IINE L1241 支持。玩家请使用 PC 无线模式，进入配对状态后，在配对列表选择 `IINE GAMEPAD`。27.02 是已有实测版本；相同型号的其他版本显示“版本未验证”，需按机身 OK 尝试连接，60 秒未确认则断开。确认仅对本次连接有效，重连需返回配对页再次确认。其他型号和异常身份应答仍拒绝。方向键或左摇杆控制方向，A/B 对应游戏 A/B，减号/加号对应 Select/Start。

主页“游戏管理”下方新增“手柄测试”，设置页不再保留入口。显示双摇杆原始统一坐标、方向箭头、A/B/X/Y、肩键、扳机、底部 Select / Start 和报告计数；所有按键只显示当前按下状态，松开后下一帧直接复原。L3 / R3 不再单独显示，按下摇杆使对应圆形区域变色，松开立即恢复；移除 Home/截图显示。长按机身 OK 返回，Select 长按 1 秒返回，其他手柄输入仅显示。布局参考贡献者实时界面，保留 MIT 署名，使用 FC 红白配色及既有 LCD 条带缓冲。不测试震动、陀螺仪、Turbo 或校准。扳机显示收到的值，已测 L1241 仅有数字端点。

模式名称不等于传输协议。[厂家指南](https://iine.store/pages/pocket-controller-ns-guide-manual)列出 Android 配对+A、iOS 配对+Y、PC USB；未说明 PC 无线协议。[贡献者记录](https://github.com/1ikeMcFlurry/esp32c3-iine-gamepad/blob/codex/main/docs/PROTOCOL.md)描述私有 BLE 配置接口。2026-10-12 用户向开发者确认仅 Android 模式经过实测，PC 无线待测；收回此前“两种组合均已实测”的表述。两个固件已有标准 Xbox BLE HID 解析入口，若 PC 模式提供兼容报告描述符可能直接连接；不能只凭 Xbox Wireless Controller 名称认定兼容。

来源：[1ikeMcFlurry 的 GameBoy PR #1](https://github.com/PhoenixZHC/folotoy-gameboy/pull/1)，已核对 head `fb8cc0fe5d4e6add7d06f5c1214cda9d6eef8d01`，合并提交 `bdc10f29e65d5fc740616b404fb0b8e8de25b41a`。保留协议、传输、测试代码的作者署名及 `third_party_licenses/iine-gamepad-MIT.txt`。

适配使用私有配置服务，经 BLE 安全流程与固件身份校验后启用输入，不改变标准 HID 路径。输入是增量报告，保持未变化的按键状态；断开连接或删除设备时停止计时器和通知监听，现有平台断连流程释放按键。手柄改动未访问 OTA 服务或改变分区；同轮 FC 格式与 RAM 改动另见 [NES 2.0 记录](nes2-support.md)。

## 本次验证

主页现已接入与设置页相同的手柄菜单输入：方向键／左摇杆上下选择，A 确认，机身按键保留。原先主页只读取机身队列的问题已修复。按钮及方向键释放后不再保留浅色标记，下一帧直接恢复。两项修正尚未刷入 COM14；此前设备测试使用旧包。

- FC：24 组主机测试通过、0 失败，包含 358 条采集输入报文回放、版本确认、测试页状态/绘制边界、主页菜单，以及 NES 2.0 / 网络 / 上传新增检查；ESP-IDF 5.5.3 正式构建、镜像布局、打包读回及 ELF 身份检查通过。
- GameBoy：32 组主机测试通过、0 失败，仓库检查通过；ESP-IDF 5.5.3 构建、布局及 8 MiB 清洁包检查通过。首次合并前另有 4 组补充 C 测试、20 项补充 Python 测试通过，18 个 PR 文件的 Git 内容哈希与 PR head 一致。新增页面已检查中文字体覆盖和模拟输入渲染预览，不是实机截图。
- WSL 门禁因 `HCS_E_HYPERV_NOT_INSTALLED` 无法启动；上述检查使用 Windows 原生工具完成。未运行 actionlint；工作流文件未修改。合并前 GitHub 两项工作流为 `action_required`，不记录为通过。
- GameBoy 第一次原生测试有两个新程序遇到 Windows `WinError 5`，再次运行全部 30 组通过。首次镜像合并因输出相对路径错误失败，改为绝对路径后通过；失败产物未作为交付。
- 后续已按用户授权将完整包写入 COM14，校验及启动通过。用户确认 PC 无线模式下 IINE GAMEPAD 已连接、测试页按键有反应；串口确认同名 BLE gamepad。此结果限定该设备的基本连接/输入，不等于 Xbox 标准 HID、全部控件或长期稳定性通过；见 [设备记录](device-pc-wireless-test.md)。GameBoy 本轮未烧录。

## 本地产物

- FC 应用（主页/释放显示修正版，未烧录）：`build/FoloToy-FC.bin`，SHA256 `9df1c2468717a1d00412d047115d65521c8d800aecf813b57625d69f8d69d894`。
- FC 清洁完整包（同上）：`build/release/FoloToy-FC-full.bin`，SHA256 `bcca2151d5259d9197f27e004a18b1e81b21dc4a82703eed3b192c9f2c4f05e1`；同目录保留匹配的 ELF、MAP、配置和校验清单。
- GameBoy 应用：`E:/code/folotoy-gameboy/build/l1241-validation/FoloToy-AI-Passport.bin`。
- GameBoy 清洁完整包：`E:/code/folotoy-gameboy/build/l1241-validation/FoloToy-GameBoy-L1241-8MB-clean.bin`，SHA256 `33e2bd7a128be4641275d1355a1650d462e2437eaec9a8cc04aca80f8e4081f7`；匹配 ELF、MAP 和配置位于同一构建目录。

应用镜像仅用于分区兼容的 `0x10000` 更新；完整清洁包从 `0x0` 写入会覆盖游戏、存档和设置。FC 完整包已按后续授权写入 COM14，当前为空游戏状态；GameBoy 未烧录。历史预装包未更新，固件版本号保持 FC 1.3 / GameBoy 1.7。未创建分支、发布固件或推送源码修改。

待实机确认：初次配对、方向/A/B/Select/Start、组合键与长按、断连释放与重连、息屏唤醒（GameBoy）、长期游玩和其他手柄回归。其他型号及 L1241 其他固件版本不作兼容承诺。
