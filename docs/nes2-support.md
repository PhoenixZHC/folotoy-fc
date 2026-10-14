# NES 2.0 与游戏管理补齐（2026-10-12）

> 当前 FC 1.5 完整包与设备身份见 [交付说明](release-1.5.md)，验收边界见 [验证记录](validation.md)。本页版本、哈希和测试结论保留其历史时点或专题范围。

## 当前验证边界（2026-10-12）

这些功能已包含在 COM14 的 FC 1.4 中，24 组主机测试及构建通过；现有游戏启动不等于 NES 2.0 诊断 ROM、手机上传拒绝流程或断电存档实机验收。其余历史实现说明如下。

对应用户截图中未遮挡的功能；不扩展模拟核心的 Mapper 范围，不改分区或历史发布包。源码支持不代表手机、手柄或所有 ROM 已实机验收。

## 格式和 RAM

- iNES 保持原有兼容规则；NES 2.0 接受现有 Mapper 0、1、2、3、4、7、23、66 的既有容量范围。Mapper 4 上限仍是 PRG 1.5 MiB / CHR 256 KiB。
- 普通和指数乘数形式先解码为字节数，再检查溢出、银行对齐及 Mapper 容量。不会因新格式放宽核心不支持的尺寸。
- NES 2.0 仅接受子型号 0，NTSC 或可按 NTSC 运行的多区域声明，未指定外设或标准手柄。PAL、Dendy、特殊主机、附加 ROM、非零保留位和其他外设明确拒绝。
- PRG RAM 支持无、8 KiB 易失 RAM、8 KiB 电池 NVRAM 三种；拒绝混合及其他尺寸。电池标记须与 NVRAM 一致。无 RAM 不分配也不映射可写卡带内存，普通 RAM 不进入存档。
- CHR ROM 存在时不接受额外 CHR RAM；无 CHR ROM 时要求明确声明 8 KiB 易失 CHR RAM。CHR NVRAM 不支持。Trainer 需要声明可用 PRG RAM。
- iNES 未声明 RAM 的旧语义仍推定 8 KiB，避免把旧游戏错误改成无 RAM。

规则参照 [NESdev NES 2.0](https://www.nesdev.org/wiki/NES_2.0)，实现位于 main/fc_rom.c。文件头仅用于声明校验，不能证明游戏一定兼容。

## 上传与网络

上传先接收完整 16 字节头，再用与加载完全相同的 fc_rom_validate 检查格式和声明长度。检查通过才解除映射、擦除空闲区和写入。格式拒绝不会产生 Flash 擦写；传输中断则不提交游戏目录。网页显示具体中文错误，提前拒绝时关闭尚有未读请求体的连接，避免后续请求错位。打包器与固件解析器以 810 组输入交叉检查。

热点名 `folotoy-fc`（无密码），显式配置 192.168.4.1/24 和 DHCP，机身显示完整 IP `192.168.4.1` 及“无互联网也保持连接”；浏览器访问 `http://192.168.4.1/`。网页禁用缓存并提示关闭连接。退出时按 HTTP、Wi-Fi、事件处理器、网卡顺序清理；失败保留资源句柄，下次启动先重试，避免丢失句柄或重复创建。主机故障注入覆盖各阶段失败和重复进出；手机浏览器、蜂窝网络切换及实际网络资源释放仍需设备验证。

原有文件头/容量/Mapper 检查和 16 kHz 音频适配保留；不另做音源替换或音频重构。

## 原创诊断文件

用项目 Python 生成，不包含商业游戏或 BIOS 数据：

```powershell
& '.\.venv\Scripts\python.exe' tools/make_nes2_diagnostic.py --output build/diagnostic-nes2.nes
& '.\.venv\Scripts\python.exe' tools/make_nes2_diagnostic.py --exponent --output build/diagnostic-nes2-exponent.nes
```

两份文件的 PRG/CHR 完全相同，仅容量声明形式不同。上传后检查：

1. 顶部红、绿、白色条；A/B/SE/ST/UP/DN/LT/RT 下方显示 0/1。
2. 按住 A 发声，松开停止发声。SE/ST 对应 Select/Start。
3. Select 每次按下将 SAVE MARK 加一（0–9），按住不重复增加。
4. 正常退出游戏触发保存，再打开应显示 LOADED SAVE 并保留数字；首次启动显示 NEW SAVE。

主机通过实际 Nofrendo 核心执行普通／指数两个诊断文件，检查按键、PPU 内容、声音输出和 SRAM 导出／导入后的标记。真实 LCD 色彩、喇叭、Flash 保存及断电恢复仍待实测。
