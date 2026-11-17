# ZHIoT_Sensor_Env —— 大气压 / 温湿度 / 光强 / 二氧化碳 + 云端控制

本工程把原来分开的四个传感器实验（大气压 CPS121、温湿度 SHT30、光照 BH1750、二氧化碳 SGP30）合并到一个工程里，
统一走 **I2C2 总线**读取，并沿用「电子围栏场景实验」的联网部分（LiteOS + ESP8285 + 华为云 OC 协议），
实现 **数据上报云端 + 云端下发指令控制灯和风扇**。

工程已用 Keil MDK（ARMCC V5.06，STM32L431VCTx）完整编译通过：**0 Error，0 Warning**。
编译产物：`MDK-ARM\ZHIoT_Sensor_Env\ZHIoT_Sensor_Env.hex`（可直接烧录）。

---

## 一、硬件连接

| 功能 | 引脚 | 说明 |
| --- | --- | --- |
| I2C2_SCL | PB13 | 四个传感器共用 |
| I2C2_SDA | PB14 | 四个传感器共用 |
| CPS121 大气压 | I2C 地址 0x6C | 写 `30 0A` 启动测量，读 `06` 寄存器 3 字节 |
| SHT30 温湿度 | I2C 地址 0x44 | 写 `24 00` 单次测量，读 6 字节（含 CRC 校验） |
| BH1750 光照 | I2C 地址 0x23 | 写 `01` 上电 + `10` 连续高分辨率，读 2 字节 |
| SGP30 二氧化碳 | I2C 地址 0x58 | 写 `20 03` 初始化 + `20 08` 测量，读 6 字节（CO2 ppm + TVOC ppb，各带 CRC） |
| 灯 | PB15 | TIM15_CH2 输出 PWM（20 kHz），档位 0~100 调光 |
| 风扇 | PA8 | 普通推挽输出，默认高电平开（见下方宏） |

> SHT30 使用的是板上的 I2C 温湿度芯片（电路图里的 SHT30-DIS-B），不是 Cps121 参考工程里的 DHT11 单总线方案。
> 如果实测发现风扇是低电平有效，把 `Hardware/ENV/env_control.h` 里的
> `ENV_FAN_ACTIVE_HIGH` 从 1 改成 0 即可。

---

## 二、程序结构

```
ZHIoT_Sensor_Env
├─ Hardware/ENV
│   ├─ env_sensor.c / env_sensor.h     四个传感器的 I2C 读取（共用 I2C2）
│   └─ env_control.c / env_control.h   灯（PB15 PWM）、风扇（PA8）控制
├─ task_user
│   ├─ env_task.c / env_task.h         新增：每 2 秒采样一次，填充云端上报数据
│   ├─ main.c                          改：创建 env_task（替代原围栏任务）
│   ├─ control_task.c                  改：联网处理 + 云端下发指令解析
│   ├─ view_task.c                     改：屏幕显示温湿度/气压/光照/CO2/设备状态
│   └─ task_inter.h                    改：界面数据结构（去掉围栏字段）
├─ Components / Src / api / Liteos ... 沿用电子围栏工程（联网、协议、GUI、HAL）
└─ MDK-ARM/ZHIoT_Sensor_Env.uvprojx    工程文件（已加入新文件与新头文件路径）
```

任务划分：

| 任务 | 优先级 | 周期 | 职责 |
| --- | --- | --- | --- |
| control_task | 0 | 10 ms | 协议发送/心跳、ESP8285 状态机、串口转发、云端指令执行 |
| view_task | 1 | 10 ms | 屏幕刷新（500 ms / 2 s 分级刷新）、按键扫描 |
| env_task | 2 | 2 s | 读四个传感器 → 填入协议结构体 → 触发上报 |

数据流：`env_task` 采样 → `g_pltSendData`（农业场景结构体）→ `ptl_Heartbeat_Handle` 触发 →
`ocprotocol.c` 打包成 JSON → ESP8285 通过 MQTT 发布到 OC 平台；
云端下发 → ESP8285 解析 `paras` 成十六进制串 → `ptl_Receive_OCData_Handle` → 控制 PB15 / PA8。

---

## 三、云端上报字段（农业场景 Farm）

| 平台字段 | 内容 | 示例 |
| --- | --- | --- |
| F_Temperature_Input | 温度 ℃，1 位小数 | "25.6" |
| F_humidity_Input | 湿度 %RH，1 位小数 | "60.3" |
| F_pressure_Input | 气压 hPa，整数 | "1012" |
| F_light | 光照 lx，整数（上限 9999） | "350" |
| F_Carbon_dioxide_Input | 二氧化碳 ppm，整数（上限 9999） | "620" |
| F_fill_light_switch | 灯状态 0/1 | 1 |
| F_illumination_adjust_Input | 灯亮度档位 0~100 | 60 |
| F_irrigation_switch | 风扇状态 0/1（占用灌溉电机通道） | 1 |
| F_enclosure_Output | 固定 0（本实验没有围栏） | 0 |
| Error_msg | 异常码 | 0 |

协议字段长度是 `char[5]`，只能放 4 个字符，所以温度/湿度做了 99.9、-40 的上下限保护，
光照超过 9999 按 9999 上报。

## 四、云端下发指令（控制灯和风扇）

ESP8285 会把下发的 `paras` 拼成十六进制串，本工程按下面 5 个字节解析：

```
Header(06) + F_FunCode(00) + 补光灯开关(1B) + 补光灯档位(1B) + 灌溉/风扇开关(1B)
```

示例：

| 下发数据 | 含义 |
| --- | --- |
| `0600013200` | 灯开、亮度 50%、风扇关 |
| `0600000000` | 灯关、风扇关 |
| `0600016401` | 灯开、亮度 100%、风扇开 |

上电后灯默认关闭、档位 60，风扇默认关闭；收到指令后立即执行，并马上把新状态回报云端
（屏幕「设备状态」一栏也会同步变化）。

---

## 五、使用前必须改的配置

配置都在 `Components/esp8285/esp8285.h`：

```c
#define SERVER_IP   "121.36.42.100"   // 华为云 IoT 平台地址
#define SERVER_PORT 1883
#define WIFI_SSID   "OPPO"            // ← 改成你现场的路由器名
#define WIFI_PSWD   "YOUR_WIFI_PASSWORD"        // ← 改成你的 WiFi 密码
#define PROD_ID     "655c46d4...324"  // ← 改成你自己的产品/设备标识
#define USER_PSWD   "d9d0f315...71912"// ← 改成你自己的设备密钥
#define TIMESTAMP   "2023011215"      // ← 与平台侧保持一致
```

`PROD_ID` 等参数沿用的是参考工程示例设备。多人同时用同一套 ID 会互相抢 MQTT 连接，
上课时务必换成自己账号下创建的产品。

---

## 六、编译、烧录与调试

1. 双击打开 `MDK-ARM\ZHIoT_Sensor_Env.uvprojx`；
2. `Rebuild`（F7 全编译）→ 0 Error；或直接用已生成的 `ZHIoT_Sensor_Env.hex` 烧录；
3. 下载：ST-Link（SWD）→ Keil `Download`，或用 STM32CubeProgrammer 烧 hex；
   - 如果报 `Error: Flash Download failed - 'Cortex-M4'`：本工程是从电子围栏版复制的，
     原来的下载算法挂在 ULINK2（`UL2CM3`）驱动名下，把调试器切成 ST-Link 后它名下没有算法，
     就会出这个报错。本工程已补好 ST-Link 的配置（SWD + `STM32L4xx 256KB Flash`，
     `0x08000000` / `0x40000`），打开工程直接点下载即可（已实测 Erase/Program/Verify 全部通过）。
   - 换电脑或换工程后若复现，按 `Options → Debug → Settings` 检查：`Port` 选 **SW**、
     勾 **Connect under Reset**、`Flash Download` 页要有 `STM32L4xx 256KB Flash`。
   - 还不行就注意供电：板子带 ESP8285，只靠 ST-Link 的 3.3V 容易欠压失败，
     给板子单独供电、ST-Link 只接 SWDIO/SWCLK/GND；必要时用 STM32CubeProgrammer 的
     `Full chip erase`（Under Reset 模式）解掉读保护后再下载。
4. 串口调试：**串口 2（DEBUG_UART = uart2），115200 8N1**，可看到采样打印与云端指令打印；
   按 KEY3 可以打开/关闭调试打印（`debugflag`），此时会把 ESP8285 的 AT 交互也打印出来；
5. 屏幕显示：温度 / 湿度 / 气压 / 光照 / CO2 → 设备状态（灯、风扇）→ 模块信息（联网状态）。

---

## 七、已规避的几个坑

- **MicroLIB 不支持 `%f`**：工程 `useUlib=1`，所以温度和湿度统一用「整数+小数位」手工格式化，
  没有使用 `printf("%.1f")`；
- **CPS121 读寄存器时序**：用 `HAL_I2C_Mem_Read`（写地址 + 重复起始 + 读数据），
  而不是「写一次 + 停 + 再读」，避免读回无效值；
- **SHT30 数据校验**：6 个字节里带 2 个 CRC8，校验不过就判为本次读取失败；
- **SGP30 上电要等**：`20 03` 初始化后要等 15 ms 以上；刚上电约 15 秒内输出的是默认基线值（约 400 ppm），
  这段时间数值偏“干净”是正常的，之后才逐渐反映真实空气质量；
- **SGP30 采样率**：datasheet 建议 1 秒一次，本工程是 2 秒一次，数值仍然有效，只是响应稍慢；
- **协议字段只有 4 个字符**：温度/湿度/光照/CO2 都做了限幅，避免出现 `"100."` 这种被截断的字符串；
- **传感器读取失败**：屏幕显示 `---`，云端保留上一次的有效值，不会把 0 报上去；
- **文件编码**：新增/修改的 `.c/.h` 都是 GBK（与参考工程工程一致），用记事本另存时注意不要存成 UTF-8。

## 八、和四个参考工程工程的差异

| 项目 | 原有做法 | 本工程 |
| --- | --- | --- |
| 大气压 | Cps121 参考工程：软件模拟 I2C（PB13/PB14 翻转 IO） | 硬件 I2C2（PB13/PB14），读法与时序对齐数据手册 |
| 温湿度 | Cps121 参考工程：DHT11 单总线 | SHT30，走 I2C2，带 CRC 校验 |
| 光照 | Illumination 参考工程：软件模拟 I2C + PD12 使能 | 硬件 I2C2，与另外几路共用总线 |
| 二氧化碳 | CO2 参考工程：SGP30，软件模拟 I2C | 同款 SGP30，改成硬件 I2C2，带 CRC 校验，并入同一采样周期 |
| 联网 / 云端 | 只有电子围栏工程有（LiteOS + ESP8285 + OC 协议） | 直接复用该协议栈，业务换成环境采集 |
| 执行器 | 电子围栏：激光测距 + 蜂鸣器报警 | 灯 PB15（PWM 调光）+ 风扇 PA8（云端可控） |
