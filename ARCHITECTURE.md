# 架构

`game_main.c` 负责正式菜单、配对、游戏循环和暂停。`gamepad.c` 通过 Bluepad32/BTstack 提供 BLE 输入，`gb_input.c` 映射 NES 按键。测试构建由 CMake 的 `FC_SIM_TEST` 选择独立 `tests/fc_sim_main.c`，不改变正式入口行为。

`fc_rom.c` 验证经典 iNES 头、Mapper、ROM 长度及银行数。`gb_storage.c` 管理 4 MiB Flash 分区：4 KiB 对齐，SHA256、双目录提交及只读 mmap。工厂容器 `FFCR`，可写目录 `FFC2`，均区别于参考 GB 格式。`tools/pack_roms.py` 生成并验证同格式容器。

`fc_port.c` 维持单个 Nofrendo 会话，Flash 映射供 PRG/CHR ROM 只读访问；堆中保存 272×240 字节帧缓冲、调色板及卡带 RAM。`nes.c` 驱动 CPU/PPU/APU/Mapper，`gb_display.c` 将有效 256×240 区域缩放到 LCD，`gb_audio.c` 将每帧 266/267 个样本排队送至 ES8311。

FC 显示复用现有显示任务、条带 DMA 缓冲和完成信号量，不分配第二份完整帧。主循环只在显示空闲时渲染，提交后借出核心帧缓冲与调色板；后台任务逐条传输，其间主循环只能执行不写帧缓冲的模拟帧。菜单绘制等待传输完成；所有会话销毁路径先 drain，避免释放仍被显示任务读取的内存。传输错误向主循环返回；display_fps 由后台成功完成次数计算，emu_fps 表示 CPU/APU 帧推进。PassportSim 测试覆盖提交/完成握手，主机测试覆盖跳帧不修改帧缓冲；实际性能仍需真机确认。

`gb_saves.c` 用 ROM SHA256 区分 8 KiB 电池 SRAM，两个文件副本各带序号/CRC，较新副本损坏时恢复旧副本，双坏时报告错误；不自动格式化存档。网页 `gb_web.c`/`gb_manager.html` 仅在管理模式操作游戏目录。

Flash：NVS `0x9000/0x6000`，PHY `0xf000/0x1000`，应用 `0x10000/0x300000`，ROM `0x310000/0x400000`，存档 `0x710000/0xf0000`。完整镜像覆盖 8 MiB，属于全新安装镜像。

核心热路径：APU 四类相位用 Q16 定点；仅 CPU 解释器开启 GCC switch 跳转表；PPU 不绘制且无 latch 回调时只处理精灵 0，已命中即返回，有回调时保留原路径。绘屏按距上次实际绘制帧数调度；原跳帧命中近似算法未改变。

测试包括主机实际 C 核心、522 个历史 ROM 烟测、16 个交付 ROM 烟测、PassportSim 测试入口及真机串口。正式无线入口仍不受模拟器支持；实际显示性能已有真机数据，但全场景超过 15 fps 和长期稳定性尚未通过，见 `docs/performance.md`。
