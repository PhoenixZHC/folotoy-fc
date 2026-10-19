# Q36 D 档连接排查（2026-10-19）

## 最终用户验收与 1.6 交付

用户已确认关闭诊断日志的日常版连接、按键和关机回连正常；另确认良值口袋手柄迷你、良值口袋手柄 Pro 的安卓和 Xbox 模式均可正常使用。后两只精确硬件/固件版本未提供，不能仅凭广播名把日志逐条对应到型号。

后续监控 `.work/q36-diag/serial-next-controller-20261019.log` 确认 Xbox BLE（045e:0b13）、通用安卓（4353:9b09）以及 IINE L1241 FW27.02 路径进入 ready；保留建链失败0x3e后重试成功、一次未知/截断报告、IINE超时0x08断开与再次ready的现象。是否为主动关机测试未独立确认，不扩大为耐久验收。

上述验收针对版本标记1.5的修复日常版；版本及网页标识已升级1.6并生成原16款完整包，新包未再烧录，见[1.6交付](release-1.6.md)。以下排查过程中的“待反馈”为当时状态，现已由本节用户反馈补齐。

## 修复结论

已修复 BTstack 的 CCC 描述符分页发现状态机：找到 CCC 后立即保留写入状态，避免后续分页逻辑覆盖；查询到末尾仍没有 CCC 时返回 `ATT_ERROR_ATTRIBUTE_NOT_FOUND`，不再悬挂请求。

实机证明触发条件：第一组查询范围 `0x0017..0x0019`，Q36 首包仅返回到 CCC `0x0018`；第二组范围 `0x001b..0x001d`，首包返回到 CCC `0x001c`。旧代码找到 CCC 后又继续查询后面的 Report Reference，最后停在等待状态，导致 20 秒超时。修复版两次订阅均成功，选择时间 `27595 ms`，`controller ready` 为 `29716 ms`，约 2.1 秒。用户已确认可以连接并使用按键。

该修复适用于相同 CCC 分页返回方式的 BLE 外设，不是按 Q36 名称放行，也没有更改映射或超时。其他手柄可尝试支持 BLE HID 的安卓模式，但“安卓模式”本身不能证明使用 BLE 或一定兼容。X 档经典蓝牙仍超出 ESP32-C3 能力。

验证：新增测试直接提取生产代码的 ATT Find Information 分支，以模拟回复复现原错误；5 个场景覆盖分包命中、继续分页、单包命中、缺失 CCC、128 位厂商描述符。旧代码断言失败，修复后通过。27 组主机测试通过、0 失败；初轮 4 组被 Windows `WinError 5` 阻断，仅重跑这 4 组后通过，其余结果复用。一次并行构建同样受拒绝访问影响，串行构建及镜像校验通过。

原始诊断与修复版串口日志位于 `.work/q36-diag/serial-diag.log`、`.work/q36-diag/serial-fixed-diag.log`，不提交仓库。诊断开关关闭的日常版应用为 `build/FoloToy-FC.bin`，2,540,144 字节，SHA256 `974aac7f4419a19e36ed5410af75658ef34ba99a162a25d10ffd8400c80c6029`；匹配 ELF SHA256 `465b766b928d3123d76af1660b769a036f479381efadf4896a20a5e9b8325809`。日常版已编译打包并核验，仅写入 COM13 的应用分区，写入哈希校验通过。最终日志 `.work/q36-diag/serial-release.log` 确认启动 ELF 前缀匹配、`FC_READY version=1.5 games=16`，监控未见 panic/abort，串口已释放。最终日常版手柄复测及关机回连已由用户确认正常；尚未验收长时间稳定性及其他型号。

## 已确认的实物行为

- COM13 启动日志为 FC 1.5，ELF 前缀 `1bb540ff8`，16 款游戏。
- Q36 X 档连接电脑时，Windows 枚举为 `BTHENUM`，HID 服务 `0x1124`，属于经典蓝牙；ESP32-C3 不支持该连接方式。设备地址仅保留在本地诊断日志。
- D 档广播 `Q36 for Android`，Appearance `0x03c3`，确实使用 BLE。
- 连续多次收到 `Re-encryption complete, success`；设备信息为 `ShanWan BM-769`、固件 `1.00.00:1.00.00`、VID/PID `1949:0402`。
- 日志在 `Search for HID service` / `Using hids_cid` 后没有完成 HID 初始化，20 秒连接总超时触发断开，随后重连。灯常亮仅能证明连接建立，未证明按键可用。
- 界面的“连接较慢 请重试”在选择后等待 25 秒时显示，是通用提示，不是手柄返回的错误码。

## 初始诊断准备（历史）

初始日志尚不能确定 HID 初始化内部阻塞点，因此先准备诊断版。没有延长超时、跳过加密、伪造 HID 描述或改动按键映射。

新增默认关闭的 CMake 选项 `FC_HID_TRACE`。启用后打印 HID 初始化状态、GATT 请求即时返回值、服务/报告计数、特征及描述符元信息、Report Map 分片偏移/长度和异步完成状态。日志不输出配对密钥或按键报告内容；关闭选项后相关输出被预处理移除。

独立诊断配置位于 `.work/q36-diag/sdkconfig`，构建目录 `.work/q36-diag/build`；不覆盖 `build/` 正式产物。使用项目规定的 ESP-IDF 5.5.3 环境：

```powershell
$buildArgs = @('--ccache', '-B', '.work/q36-diag/build', '-D', 'SDKCONFIG=E:/code/folotoy-fc/.work/q36-diag/sdkconfig', '-D', 'FC_HID_TRACE=ON', '-D', 'FC_INPUT_TRACE=OFF', '-D', 'FC_SIM_TEST=OFF', '-D', 'FC_WIFI_MEMORY_TEST=OFF', 'build')
idf.py @buildArgs
if ($LASTEXITCODE -ne 0) { throw 'Diagnostic build failed' }
```

用户已明确同意本轮 COM13 应用分区的诊断版及后续修复版烧录；仅写 `0x10000` 应用，不写引导、分区表、NVS、ROM、存档或整片镜像。原日常版回退应用已另存 `.work/q36-diag/rollback/FoloToy-FC.bin`，SHA256 为 `df35db76538a7d0273525056000c3346c4d23e02f272ca540b2d8d572e0f64a0`，与原交付记录一致。

## 初始诊断版身份（历史）

诊断版已通过 ESP-IDF 5.5.3 编译、镜像 checksum/hash、分区表一致性、应用边界及内嵌 ELF 身份校验。应用大小 2,541,264 字节，写入范围为 `0x10000` 至 `0x27c6d0`（右端不包含），低于 ROM 分区起点 `0x310000`。应用 SHA256：`1ff46e68e3df26a06207f9a956615192831a81137d532fbcb8baaf4a21116a85`；ELF SHA256：`0d925761b3bed2b2829a7f2f6df2d2e7cade272811c061d38de07fd18c82d0b9`。核验记录位于 `.work/q36-diag/verification.json`。

以上身份仅对应首次诊断版；后续修复版与日常版身份不得混用。日常版交付时关闭诊断开关；重连验收仍待补齐。

## 外部参考

[Bluepad32 issue #154](https://github.com/ricardoquesada/bluepad32/issues/154) 中有使用者报告 Q36 的 D 档可用于 BLE，并提醒再次按 Pair 可能改变地址。这仅作为排查参考，不代表本实物或当前固件已通过验收。
