# 电子围栏场景实验

基于 **STM32L431VCTx + 华为 LiteOS + ESP8285** 的物联网教学实验工程集合，配套 **华为云 OceanConnect（OC）IoT 平台** 完成「设备数据上报 + 云端下发指令控制执行器」的完整闭环。

本仓库是教学/实验用途的原始工程备份，`md5` 无改动，可作为教师演示、学生实验与二次开发的基线。

---

## 一、仓库内容

```
电子围栏场景实验
├── ZHIoT_Farm_Fence/      # 电子围栏场景原始工程（围栏检测 + 照明 + 灌溉类执行器）
├── ZHIoT_Sensor_Env/      # 环境传感器合并工程（气压 / 温湿度 / 光照 / CO2）
├── keilkilll.bat          # 一键清理 Keil 编译中间文件的批处理
└── .gitignore             # 忽略 Keil 中间产物，只保留源码与工程配置
```

| 工程 | 说明 | 主要新增点 |
| --- | --- | --- |
| `ZHIoT_Farm_Fence` | 电子围栏（Farm Fence）场景完整工程：LiteOS 多任务 + ESP8285 联网 + OC 协议 + LCD/GUI + 执行器控制 | 基线工程 |
| `ZHIoT_Sensor_Env` | 在围栏工程基础上，把原来分开的四个传感器实验合并到一个工程 | 统一走 `I2C2` 总线读 CPS121 / SHT30 / BH1750 / SGP30；新增 `env_task` 每 2 s 采样上报；云端下发控制 LED（PB15 PWM）与风扇（PA8） |

### 公共技术栈

- MCU：**STM32L431VCTx**（Cortex-M4）
- 操作系统：**Huawei LiteOS**（`Liteos/` 内核、arch、CMSIS 适配）
- 网络模组：**ESP8285**（AT 指令 + MQTT），`Components/esp8285`
- 云协议：**华为 OC / OceanConnect**，`Components/oc`、`Src/net.c`
- 外设：`Drivers/STM32L4xx_HAL_Driver`、`Hardware/`（LCD、KEY、LED、DYP_R01 等）
- 编译：**Keil MDK5 + ARMCC V5.06**（AC5）

---

## 二、编译与烧录

1. 安装 **Keil MDK5** 与 **STM32L4xx 器件包**（Device Family Pack）。
2. 打开对应工程文件：
   - `ZHIoT_Farm_Fence/MDK-ARM/ZHIoT_Farm_Fence.uvprojx`
   - `ZHIoT_Sensor_Env/MDK-ARM/ZHIoT_Sensor_Env.uvprojx`
3. `Rebuild`（全量编译），确认 `0 Error`。
4. 产物：`MDK-ARM/<target>/<target>.hex`，用 ST-Link / STM32CubeProgrammer 烧录。
   - 仓库里已保留现成的 `.hex`，可直接烧录验证。
5. 想清掉所有中间文件再重新编译，可双击 `keilkilll.bat`（或删除 `.gitignore` 中列出的产物）。

> `DebugConfig/` 与 `RTE/` 是 Keil 的调试/组件配置，保留在仓库里，换机器后无需重新配置。

---

## 三、硬件连线（`ZHIoT_Sensor_Env`）

| 功能 | 引脚 | 说明 |
| --- | --- | --- |
| I2C2_SCL | PB13 | 四个传感器共用 |
| I2C2_SDA | PB14 | 四个传感器共用 |
| CPS121 大气压 | I2C 0x6C | 写 `30 0A` 启动测量，读 `06` 寄存器 3 字节 |
| SHT30 温湿度 | I2C 0x44 | 写 `24 00` 单次测量，读 6 字节（含 CRC） |
| BH1750 光照 | I2C 0x23 | 写 `01` 上电 + `10` 连续高分辨率，读 2 字节 |
| SGP30 二氧化碳 | I2C 0x58 | 写 `20 03` 初始化 + `20 08` 测量，读 6 字节（CO2 ppm + TVOC ppb） |
| 灯 | PB15 | TIM15_CH2 PWM（10 kHz），档位 0~100 调光 |
| 风扇 | PA8 | 普通推挽输出，默认高电平开启（可用 `ENV_FAN_ACTIVE_HIGH` 反转） |

任务划分（`ZHIoT_Sensor_Env`）：

| 任务 | 优先级 | 周期 | 职责 |
| --- | --- | --- | --- |
| `control_task` | 0 | 10 ms | 协议发送、心跳、ESP8285 状态机、串口转发、云端指令执行 |
| `view_task` | 1 | 10 ms | 屏幕刷新（100 ms / 2 s 分级）、按键扫描 |
| `env_task` | 2 | 2 s | 读四个传感器 → 填入协议结构体 → 触发上报 |

数据流：`env_task` 采样 → `g_pltSendData` → `ptl_Heartbeat_Handle` → `ocprotocol.c` 打包 JSON → ESP8285 经 MQTT 发布到 OC 平台；云端下发 → ESP8285 解析 `paras` 为十六进制串 → `ptl_Receive_OCData_Handle` → 控制 PB15 / PA8。

云端字段（农业 Farm 场景）：`F_Temperature_Input`、`F_humidity_Input`、`F_pressure_Input`、`F_light`、`F_Carbon_dioxide_Input`、`F_fill_light_switch`、`F_illumination_adjust_Input`、`F_irrigation_switch`、`F_enclosure_Output`、`Error_msg`。

---

## 四、第三方组件来源

`Liteos/`（华为 LiteOS，含 `THIRD PARTY OPEN SOURCE SOFTWARE NOTICE.docx`）、`Drivers/CMSIS`、`Drivers/STM32L4xx_HAL_Driver`（ST 官方）、`Components/cJSON`（Dave Gamble）、`Components/QRENCODE`、`Security/DES` 等均为各自开源许可下的第三方代码，版权归原作者所有，此处仅作教学打包分发。

---

## 五、说明

- 本仓库已通过 `.gitignore` 排除 `*.o / *.crf / *.axf / *.map` 等约 300 MB 编译中间产物，只保留源码、工程文件与 `.hex`，克隆后直接在 Keil 里 `Rebuild` 即可。
- **联网参数已脱敏**：`Components/esp8285/esp8285.h` 中的 `WIFI_PSWD`、`USER_PSWD` 已替换为占位符（`YOUR_WIFI_PASSWORD`、`YOUR_DEVICE_SECRET`），`PROD_ID` / `TIMESTAMP` / `SERVER_IP` 仍保留实验原值以便对照。要真正跑通联网，请在上电前改回你自己环境的 WiFi 密码与 OC 设备密钥；`task_user/oc_config.h` 里的 `ENABLE_FARM=1` 决定上报的协议场景。