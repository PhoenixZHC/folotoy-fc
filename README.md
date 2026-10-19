# FC模拟器 1.6

## 当前完整交付：三方向版（2026-10-19）

用户已确认竖屏、横屏、横屏翻转及默认保存测试通过，所有菜单仍竖屏。最新完整BIN：[FC-1.6-orientation-16games-full.bin](deliverables/FC-1.6-orientation-16games/FC-1.6-orientation-16games-full.bin)，8MiB、原16款游戏，0x0安装会覆盖设备数据。应用与COM13已验收版一致；29组主机检查、7项打包检查通过。见[交付与SHA256](docs/release-1.6-orientation.md) / [English](docs/release-1.6-orientation.en.md)。以下旧测试阶段与旧包信息保留历史，不代表当前待办或最新包。

## 游戏横竖屏测试版（2026-10-19）

首版横竖屏已由用户确认全部通过。后续增加“横屏翻转”：暂停菜单和默认游戏方向均循环竖屏、横屏、横屏翻转；翻转时设备顺时针横握，所有菜单仍竖屏。新增翻转功能待实机验收，精确身份见下方说明。

仅游戏画面支持横屏，全部菜单仍为竖屏。暂停菜单第六项切换当前游戏方向；系统设置第四项保存下次启动游戏的默认方向，初始为竖屏。横屏时设备逆时针横握。COM13已仅更新应用、启动16款游戏确认；29组检查通过，实机体验待用户反馈。先前完整1.6交付包保持不变，不含本功能。见[横竖屏说明](docs/game-orientation.md) / [English](docs/game-orientation.en.md)。

## 1.6 手柄连接修复与完整包（2026-10-19）

用户确认 Q36 D 档连接、按键和关机回连正常；良值口袋手柄迷你、良值口袋手柄 Pro 的安卓和 Xbox 模式均可使用。档位名称不等同于协议，Q36 本实物 X 档是经典蓝牙，C3 仍不支持。

最新完整包为 `deliverables/FC-1.6-16games/FC-1.6-16games-full.bin`，8 MiB，原 16 款游戏，从 **0x0** 完整安装会覆盖游戏、存档、设置和配对。27 组主机检查通过、0 失败（复用同功能代码结果），7 项打包检查通过、0 失败；1.6 构建及组件/ELF/游戏一致性校验通过。实机验收对应 COM13 的 1.5 修复日常版；1.6 新包仅更新版本标识，尚未烧录。见 [1.6 交付](docs/release-1.6.md) / [English](docs/release-1.6.en.md)，[中文使用说明](docs/release-1.6-form.md) / [English](docs/release-1.6-form.en.md)。以下旧版本段落按历史日期保留，“当前/最新”不代表 1.6 身份。

## Q36 安卓模式连接修复（2026-10-19）

已修复 BLE 订阅按键通知时的 CCC 分页发现错误。ShanWan Q36 的 D 档 `Q36 for Android` 已在 COM13 完成连接，用户确认按键可用；X 档使用经典蓝牙，ESP32-C3 不支持。其他手柄可尝试支持 BLE HID 的安卓模式，但尚未逐型号验证。27 组主机测试通过、0 失败，日常版关闭 HID 诊断日志。原 16 款游戏完整包保留历史身份，本次应用更新与实机证据见 [Q36 修复记录](docs/q36-connection.md)。

## 小米遥控器横握（2026-10-14）

已恢复统一的原手柄测试布局，所有外接设备共用，未提供的按键保持不亮。实物 RC001 / 固件 2671 的按键无响应已修复，串口确认八键及释放，用户确认单键正常。**实测不能同时移动和开枪**：遥控器报告最多包含一个非零键码，多键限制见 [修复与实测](docs/rc001-input-fix.md)。当前 COM14 安装诊断开关关闭的日常版，主页首项已连接时为“游戏列表”，断开时为“连接手柄”；从列表回主页再进入不会自动启动游戏。用户确认菜单测试通过，见 [菜单修复与交付](docs/menu-entry-fix.md)。

FC 1.5 新增小米 BLE 遥控器横握映射：实体右→游戏上、上→左、左→下、下→右，TV→A、三横线菜单→B、主页→Start、返回→Select。返回短按保留游戏 Select，长按 1 秒暂停或页面返回。按 VID/PID `2717:32B8` 启用专用映射，普通键盘和其他手柄沿用原规则。26 组主机测试和构建/镜像校验通过，单键已实测；多键限制见上述修复记录。GameBoy 工程未修改。初始包见 [1.5 历史交付](docs/release-1.5.md)，映射见 [横握说明](docs/xiaomi-remote.md) / [English](docs/xiaomi-remote.en.md)。

## 1.4 全页面 Select 返回历史记录（2026-10-12）

游戏管理热点名称为 `folotoy-fc`（无密码）。手机连接后，在浏览器输入 `http://192.168.4.1/`；提示无互联网时选择保持连接。设备屏幕显示完整 IP `192.168.4.1`，省去协议前缀以避免截断。本次修正已烧录到 COM14，写入及启动身份校验通过，热点与显示实机操作待确认；包含此次修正的 16 款游戏完整包已生成，见 [最新交付](docs/release-1.4.md)。旧包保留。

所有原有长按 OK 返回/退出操作均支持长按 Select 1 秒，机身 OK 保留：设置（编辑时先结束编辑）、手柄测试、游戏管理、游戏列表/空列表、暂停菜单（保存成功后退出游戏）。配对取消/重试也复用该事件，但未连接或确认前没有可用手柄输入时仍需机身按键。进入页面及从子页面返回时重新建立持键基线，必须释放后再次长按，避免连续跨页退出。测试页 Select 短按用于观察，持续 1 秒返回。 FC 1.4 全页面 Select 返回版已写入 COM14，仅更新 0x10000 应用，保留 16 款游戏、存档和设置；写入哈希及启动身份校验通过。GameBoy 1.8 本批未烧录。全页面长按实际操作仍待用户反馈。


<img src="assets/project-gallery/01-fc-cover.png" alt="FC模拟器红白机主题宣传封面，非实机截图" width="360">

源码仓库：[PhoenixZHC/folotoy-fc](https://github.com/PhoenixZHC/folotoy-fc)。仓库包含源码、测试工具、文档及宣传图，**不包含商业游戏 ROM、预装游戏固件、存档或设备日志**。下文 `build/`、`deliverables/` 和 `.work/` 是本地生成路径，不是仓库下载链接。

ESP32-C3 / FoloToy AI Passport 的独立 FC/NES 固件，基于 GameBoy 框架并使用 Nofrendo 核心。**1.5** 新增小米遥控器横握映射，保留 1.4 的 L1241 手柄支持、主页手柄导航和测试页、长按 Select 暂停，以及受限 NES 2.0 和擦写前上传校验；保留 1.3 启动/音频修复、Mapper 4 程序 ROM 1.5 MiB 上限、红白界面和彩色游戏。容量通过不代表全部 ROM 兼容。

最新带原 16 款游戏的菜单修复日常版完整 BIN 为 `deliverables/FC-1.5-menu-entry-fix-16games/FC-1.5-menu-entry-fix-16games-full.bin`，8 MiB，从 `0x0` 完整安装；会覆盖游戏、存档、设置和配对。校验值、使用范围和当前设备身份见 [1.5 完整交付](docs/release-1.5.md) / [English](docs/release-1.5.en.md)，菜单验收见 [修复记录](docs/menu-entry-fix.md)。交付复核通过，本轮未再次烧录。默认 `build/release/FoloToy-FC-full.bin` 仍为空游戏版，与本地预装包区分；历史包保留。

当前变更与验证见 [横握说明](docs/xiaomi-remote.md)，历史版本见 [中英文更新日志](CHANGELOG.md)。

发布表单可复制的使用方法与本次更新日志：[中文](docs/release-1.5-form.md) / [English](docs/release-1.5-form.en.md)。

当前 FC 27 组主机测试通过、0 失败，构建与镜像校验通过；COM13 已安装 FC 1.5 Q36 修复日常版，原 16 款游戏，启动与 ELF 匹配。用户确认 Q36 修复版连接及按键可用，最终关闭诊断日志版本的手柄复测及关机回连待反馈；监控结束，串口释放。此前单键和菜单流程已验收，遥控器无法同时移动/开枪；星之卡比最长 155 ms 音频等待、3 次音频丢块和 2 个积压输入丢弃仍未定位。Q36 证据见 [修复记录](docs/q36-connection.md)，历史启动修复见 [回归记录](docs/wifi-memory-regression.md)。

## 功能与边界

- 运行经典 NTSC iNES 和受限 NES 2.0 `.nes`，已接入 Mapper **0、1、2、3、4、7、23、66**。NES 2.0 支持范围内的普通／指数容量声明，接受 NTSC 或多区域、子型号 0、标准手柄配置。Mapper 4 程序 ROM 上限为 **1.5 MiB**，图形 ROM 上限 256 KiB；容量通过不代表所有变种兼容。拒绝 PAL、Dendy、FDS、异常长度和不支持的硬件声明，不修改用户 ROM。格式和诊断文件见 [NES 2.0 记录](docs/nes2-support.md)，历史容量实测见 [测试记录](docs/mapper4-capacity-test.md)。
- 按用户选择保留 240×320 竖屏，256×240 游戏画面等比缩放为 240×225，不裁切、不拉伸铺满；16 kHz 单声道 APU。LCD 后台传输期间只推进不改写帧缓冲的 CPU/APU 模拟帧；串口分别报告 emu_fps 与实际完成传输的 display_fps。
- 选中游戏且 ROM/存档校验成功后，播放约 1 秒 FC 卡带插入动画，再启动游戏；设备开机不播放。本轮启动回归包含动画和 LCD，用户已确认口袋妖怪运行正常；不代表全部游戏画面验收。
- 沿用 BLE 键盘/手柄输入、板载菜单按键、暂停、音量/亮度设置及 Wi-Fi 网页游戏管理。
- 源码新增良值 IINE L1241（固件 **27.02**）私有 BLE 适配，来源为 [GameBoy PR #1](https://github.com/PhoenixZHC/folotoy-gameboy/pull/1)，保留作者署名和 MIT 许可。请使用 PC 无线模式，进入配对状态后，在列表选择 `IINE GAMEPAD`；方向键／左摇杆、A/B、减号／加号对应方向、A/B、Select/Start。27.02 正常连接；同型号其他固件显示版本和“版本未验证”，按 OK 才尝试，长按 OK 取消，60 秒无确认则断开。确认仅对本次连接有效，其他型号仍拒绝。主页“游戏管理”下方新增“手柄测试”：显示双摇杆、方向、按键、扳机值和实时按下状态，长按机身 OK 返回；不发送震动、宏或校准命令。用户已在 FC 的 PC 无线模式确认 IINE GAMEPAD 连接及测试页按键正常；重连、其他固件和长期稳定性待验收，不能据此认定 Xbox 标准 HID 已验证。详见[适配记录](docs/l1241-support.md)。
- 键盘映射：WASD 移动、J=A、K=B、I=Select、L=Start；手柄使用十字键或左摇杆与 A/B；左摇杆采用 ±160 死区，同轴十字键优先。方向与攻击可同时保持。游戏输入缓存最多 16 次状态变化、最长 100 ms，逐模拟帧消费；暂停/断连清空，进入和恢复先松键。超限记录丢弃，不保证极密输入或长卡顿不漏按。1.1 包已包含修复；本轮手柄实测反馈正常，实体键盘和更多型号仍需专项验证。
- 最多 16 个游戏，共用约 4 MiB ROM 分区；每个名称最多 31 个 UTF-8 字节。ROM 直接映射 Flash，不依赖 PSRAM。8 KiB 电池 SRAM 双副本存档；没有即时存档功能。
- NES 2.0 明确区分无 PRG RAM、8 KiB 普通 RAM 和 8 KiB 电池 RAM；仅电池 RAM 保存。上传先读完整文件头，用与加载相同的规则检查，拒绝时显示具体原因且不擦写。热点固定为 `http://192.168.4.1/`，退出清理失败时保留句柄供重试；手机提示无互联网时需保持热点连接。
- 16 款完整包已占满槽位，包含《武林外传》，游戏区剩余 4 KiB，独立保留 960 KiB 存档分区。清单及验证见 [16 款安装记录](docs/games-full16.md)。1.5 预装包沿用原游戏分区，已全新写入 COM14，见 [烧录记录](docs/release-1.5.md)。
- ESP32-C3 只有 BLE，不支持蓝牙经典协议手柄。已有游玩和输入变化日志；完整配对/断连重连、Wi-Fi 上传仍待专项验收，实际游戏速度见性能报告。
- 原框架 `gb_` 内部文件/接口名暂保留，编译使用的模拟核心是 `components/nofrendo`。旧启动动画文件不参与构建。

## 构建

环境：Windows PowerShell、ESP-IDF **5.5.3**、ESP32-C3、8 MiB Flash。本机 IDF 固定路径 `C:\esp\v5.5.3\esp-idf`。主机测试另需 PATH 中的 GCC。首次建立项目 Python 环境：

先克隆仓库并进入目录：

```powershell
git clone https://github.com/PhoenixZHC/folotoy-fc.git
Set-Location folotoy-fc
```

下面的 `tools/build.ps1` 使用开发机固定 IDF 路径。其他电脑先在自己的 ESP-IDF 5.5.3 PowerShell 环境中激活工具链，再执行 `idf.py set-target esp32c3`、`idf.py -B build -DFC_SIM_TEST=OFF build`，最后用项目 Python 执行 `tools/package_firmware.py`；无需本地商业 ROM 即可构建默认空游戏版本。第三方核心、BLE 和 BSP 源码随仓库保留，托管组件按 `dependencies.lock` 下载。

```powershell
py -m venv .venv
.\tools\build.ps1
& '.\.venv\Scripts\python.exe' tools/verify_firmware.py build
```

默认镜像在 `build/release/FoloToy-FC-full.bin`，8,388,608 字节，附同版本 ELF、MAP、sdkconfig 和 SHA256 清单。打包会验证 app 内嵌的 ELF SHA256、分区边界及写后回读。默认镜像**不含游戏 ROM**，含预格式化的空存档分区。

完整镜像从 Flash `0x0` 写入，会替换原固件及游戏/存档；1.0 已按用户授权烧录 COM14 并通过写入校验。不能把升级已有设备当作保留存档的增量更新。

## 放入自己的 ROM

启动后进入“游戏管理”，连接屏幕显示的 `folotoy-fc` 热点，用页面上传 `.nes`；设备会验证 iNES／受限 NES 2.0 头、支持的 Mapper、文件长度和 SHA256。正式版本从“连接手柄”完成 BLE 输入设备配对，已连接时主页该项变为“游戏列表”。

也可生成本地专用的预装 ROM 镜像。名称较长时通过 `--name` 指定显示名称：

```powershell
& '.\.venv\Scripts\python.exe' tools/pack_roms.py --output build/local-roms.bin --name '魂斗罗' 'FC中文游戏全集\魂斗罗 [星空汉化].nes'
& '.\.venv\Scripts\python.exe' tools/pack_roms.py --verify build/local-roms.bin
& '.\.venv\Scripts\python.exe' tools/package_firmware.py --roms build/local-roms.bin --output build/local/FoloToy-FC-local.bin
```

这些命令只生成文件，不烧录。用户 ROM、临时产物及含 ROM 的镜像不属于默认分发内容。

## 自动化测试

```powershell
& '.\.venv\Scripts\python.exe' tools/test_host.py
& '.\.venv\Scripts\python.exe' tools/test_packaging.py
& '.\.venv\Scripts\python.exe' tools/test_roms.py 'FC中文游戏全集' --frames 1800 --draw-every 2
& '.\.venv\Scripts\python.exe' tools/rom_report.py
```

报告：`build/host-tests/results.json`、`build/rom-tests/results.json`、`build/rom-tests/report.html`。`ran` 仅表示指定帧数运行结束且末帧有多种颜色，不代表完整兼容或可通关；`blank_frame` 需要单独排查；`rejected` 是当前格式/Mapper 范围外，不会被强行加载。

## PassportSim

[模拟器网站](https://passportsim.bugs.cc/) / [源码](https://github.com/BlackHole1/passportsim)。本次使用官方 Windows v1.0.1 CLI，下载包校验通过。

该版本对本项目的 Bluepad32/BTstack 镜像报告 `unsupported image`，正式入口在蓝牙寄存器访问处触发 `E_TRIPWIRE`。因此建立**独立测试入口** `tests/fc_sim_main.c`，复用实际 FC、Flash、SPIFFS、LCD、I2S 和板载按键代码，跳过 BLE/Wi-Fi；生产入口不包含测试输入。

```powershell
.\tools\build.ps1 -Simulator
& '.\.venv\Scripts\python.exe' tools/test_simulator.py '.work/simulator/passportsim-1.0.1-windows-x64/passportsim.exe'
```

模拟器入口自动运行第一个 ROM，UP=A、DOWN=右、OK=Start，按一次发送一帧输入。这只是验收入口，不是完整手柄替代。自有诊断 ROM 镜像：`build-sim/release/FoloToy-FC-diagnostic.bin`；本地魂斗罗测试镜像：`build-sim/release/FoloToy-FC-Contra-local.bin`。画面底部显示 `SIM TEST`，不可与正式版本混淆。

模拟器报告在 `build/sim-tests/results.json`，保留原始返回值和未建模寄存器清单。v1.0.1 的 `run` 有返回 `result=pass`、`status=matched` 但进程退出 10 的情况；脚本明确记录该差异，只在 JSON 验收成功且没有 tripwire 时接受它。模拟器还记录 `st7789.sleep_cycle_too_fast.0x11`，本次未据此擅改参考 BSP 的屏幕时序，需结合真机确认。

当前结果和未验收项见 `CONTEXT.md`、`docs/menu-entry-fix.md`，历史 1.4 交付见 `docs/release-1.4.md`，许可见 `THIRD_PARTY.md`。26 组主机测试通过；预装包、应用与 ELF 按记录中的哈希区分。普通构建关闭三个诊断开关，`-InputTrace` 用于输入诊断。源码使用不等于商业再分发授权。

显示性能优化采用 APU Q16 定点、CPU 指令跳转表、按实际绘制时机调度和跳帧精灵扫描精简；开源参考、前后对比方法与真机结果见 `docs/performance.md`。目标为实际显示超过 15 fps，不能以 emu_fps 或主机测试代替验收。
## 游戏暂停快捷键

游戏中长按手柄 Select 约 1 秒可打开暂停菜单，机身长按 OK 仍有效。短按 Select 保持游戏原功能，长按达到阈值前也正常传给游戏。进入或恢复游戏时已按住 Select，需要松开后重新长按，避免反复暂停。蓝牙键盘映射到 Select 的 I 键同样适用。
