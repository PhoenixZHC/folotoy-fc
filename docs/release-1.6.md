# FC 1.6 十六游戏完整包（2026-10-19）

本页保留早先手柄修复包记录。最新已验收的三方向完整包见[三方向交付](release-1.6-orientation.md)，旧包不含横屏功能，以下设备身份及“未烧录”为当时状态。

[English](release-1.6.en.md) · [使用方法与更新日志](release-1.6-form.md) · [连接修复证据](q36-connection.md)

## 交付与安装

完整 BIN：`deliverables/FC-1.6-16games/FC-1.6-16games-full.bin`，**8,388,608 字节（8 MiB），安装地址 0x0**。包含原 16 款游戏，ROM 分区逐字节不变。完整安装会覆盖游戏、存档、设置和蓝牙配对；不是保留数据的增量升级。

| 对象 | SHA256 |
|---|---|
| 完整 BIN | `06e0feec0eaff7fb96def88aac290a584b80c1744c52b6a22a0ac0a122e0ede3` |
| 应用 | `5a98eb084dfbb1dd49de1d7d62577117c4b8797cd2582884685ad6c951cbf36e` |
| 匹配 ELF | `140dda659569a51b423c3c5db39c1c79e4ccf7c9a39def5290a4735c4e05bd1e` |
| 游戏分区 | `44d35274ace0448fc0aa523d4449d9436f419705b245d272cb5974ab174d05dd` |

同目录保留组件、ELF/MAP、配置、清单、SHA256SUMS、交付核验和测试记录。原包保留；含游戏的完整包仅本地交付，本轮未提交、推送或公开发布。

## 修改与兼容范围

修复 BTstack 在分页查找 CCC 描述符时覆盖已找到的通知写入状态，避免 BLE HID 初始化卡住；末尾缺失 CCC 时明确报错。没有延长超时或按手柄名字强制放行。正式版关闭 FC_HID_TRACE、FC_INPUT_TRACE、FC_SIM_TEST、FC_WIFI_MEMORY_TEST。版本和游戏管理网页更新为 1.6，其他功能沿用 1.5。

| 实物 | 模式 | 验收 |
|---|---|---|
| ShanWan Q36 | D 档 / Q36 for Android | 串口约 2.1 秒 ready，用户确认日常版连接、按键及关机回连正常 |
| ShanWan Q36 | X 档 | 本实物使用经典蓝牙，C3 不支持；不能外推所有 Xbox 模式 |
| 良值口袋手柄迷你 | 安卓、Xbox | 用户确认两模式可正常使用 |
| 良值口袋手柄 Pro（用户称官方适配款） | 安卓、Xbox | 用户确认两模式可正常使用 |

后两只实物的精确硬件/固件版本未提供；不将广播名称或日志地址强行对应到某一型号。监控确认 Xbox BLE（VID/PID 045e:0b13）、通用安卓（4353:9b09）、IINE L1241 FW27.02 路径进入 ready。日志也有建链失败 0x3e 后重试成功、一次未知/截断报告、IINE 0x08 断开及再次 ready；断开是否由测试关机触发未独立确认，不声称无线耐久全通过。安卓/Xbox 是手柄档位名称，实际需兼容 BLE 协议；C3 仍不支持经典蓝牙。

## 验证及设备身份

- 构建：ESP-IDF 5.5.3，生产入口、8 MiB、原分区。沿用已实测 sdkconfig 并逐项核对 sdkconfig.defaults；没有配置偏差。此派生工程使用 tools/test_host.py、tools/test_packaging.py、tools/package_firmware.py 和 tools/verify_firmware.py，不含上游技能所述 validate.sh/archive_firmware.py。
- 主机：复用本轮相同功能代码已通过的 27 组、0 失败；新增 CCC 分页回归覆盖 5 个场景。初轮 Windows 拒绝访问的 4 组已单独重跑通过。
- 打包：重新运行 7 项通过、0 失败；完整布局、五组件哈希、应用版本/内嵌 ELF、四开关关闭、16 款 ROM 格式/条目哈希、原 ROM 分区一致性及回读均通过。
- 实机：上述实测对应 COM13 上版本标记 1.5 的修复日常版，应用 SHA256 `974aac7f4419a19e36ed5410af75658ef34ba99a162a25d10ffd8400c80c6029`，ELF `465b766b928d3123d76af1660b769a036f479381efadf4896a20a5e9b8325809`。COM13 已释放。
- **本次生成的 1.6 未再次烧录**；它与已测修复版的后续差异只有版本标识和网页版本文字。不能把此前实测写为此新二进制已实机验收。

长时间稳定性、其他型号/固件、手机上传、全部 Select 快捷键、NES 2.0 实机诊断及断电存档仍待专项验证；历史卡比音频/输入积压及 RC001 多键限制仍保留。本次未改 GameBoy。
