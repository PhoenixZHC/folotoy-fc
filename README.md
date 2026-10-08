# FC模拟器 1.0

<img src="assets/project-gallery/01-fc-cover.png" alt="FC模拟器红白机主题宣传封面，非实机截图" width="360">

源码仓库：[PhoenixZHC/folotoy-fc](https://github.com/PhoenixZHC/folotoy-fc)。仓库包含源码、测试工具、文档及宣传图，**不包含商业游戏 ROM、预装游戏固件、存档或设备日志**。下文 `build/`、`deliverables/` 和 `.work/` 是本地生成路径，不是仓库下载链接。

ESP32-C3 / FoloToy AI Passport 的独立 FC/NES 固件。基于原 GameBoy 项目的硬件及菜单框架，游戏核心改为 Nofrendo。版本 **1.0**：菜单、入场动画和网页采用经典红白机配色，游戏保留彩色。最终性能应用和 16 款游戏已安装 COM14，写入校验、启动和多款游戏串口实测完成；最后一轮实际显示 13.9–19.6 fps，仍有低于 15 的窗口和少量音频块丢弃，不标为全部验收通过。

完整预装 BIN、校验值和安装注意事项见 [1.0 交付说明](docs/release-1.0.md)。阶段性收尾，不代表剩余性能及兼容性问题已解决。

## 功能与边界

- 运行经典 NTSC iNES `.nes`，已接入 Mapper **0、1、2、3、4、7、23、66**。不同 Mapper 变种仍有兼容性差异。拒绝 NES 2.0、PAL、FDS、异常长度及未支持的布局，不修改用户 ROM。
- 按用户选择保留 240×320 竖屏，256×240 游戏画面等比缩放为 240×225，不裁切、不拉伸铺满；16 kHz 单声道 APU。LCD 后台传输期间只推进不改写帧缓冲的 CPU/APU 模拟帧；串口分别报告 emu_fps 与实际完成传输的 display_fps。
- 选中游戏且 ROM/存档校验成功后，播放约 1 秒 FC 卡带插入动画，再启动游戏；设备开机不播放。该动画版本已烧录 COM14 并确认启动成功，动画显示及游戏效果待用户实测反馈。
- 沿用 BLE 键盘/手柄输入、板载菜单按键、暂停、音量/亮度设置及 Wi-Fi 网页游戏管理。
- 最多 16 个游戏，共用约 4 MiB ROM 分区；每个名称最多 31 个 UTF-8 字节。ROM 直接映射 Flash，不依赖 PSRAM。8 KiB 电池 SRAM 双副本存档；没有即时存档功能。
- 本地设备已装满 16 个槽位，包含《武林外传》，游戏区剩余 4 KiB，独立保留 960 KiB 存档分区。清单及验证见 [16 款安装记录](docs/games-full16.md)。
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

启动后进入“游戏管理”，连接屏幕显示的 `FC模拟器` 热点，用页面上传 `.nes`；设备会验证 iNES 头、支持的 Mapper、文件长度和 SHA256。正式版本从“配对键盘或手柄”进入游戏列表，需要 BLE 输入设备。

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

当前结果和未验收项见 `CONTEXT.md`，依赖来源及许可见 `THIRD_PARTY.md`。使用这些源码不等于整套固件已经取得商业再分发许可。

显示性能优化采用 APU Q16 定点、CPU 指令跳转表、按实际绘制时机调度和跳帧精灵扫描精简；开源参考、前后对比方法与真机结果见 `docs/performance.md`。目标为实际显示超过 15 fps，不能以 emu_fps 或主机测试代替验收。
