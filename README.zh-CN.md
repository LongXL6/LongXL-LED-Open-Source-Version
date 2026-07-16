# LongXL LED 开源版（LongXL LED Open Source Version）

*STC8G1K08A 单键 WS2812 / NeoPixel 幻彩控制器固件*

[English](README.md) | **简体中文**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

基于 **STC8G1K08A**（8 脚 1T-8051 单片机，单价约 ¥1–2）的极简单键 WS2812 /
NeoPixel 灯带控制器固件。不用晶振、不用定时器、不用 SPI/PWM 外设 ——
800 kHz 的 WS2812 单线协议完全靠内部 35 MHz RC 振荡器 + 数周期的代码硬扛出来。

有意思的是：市面上几乎不存在真正"单键"的幻彩灯带成品控制器（最少也是
SP002E 那类三键），单键交互正是这个小设计的独特之处。

## 快速开始 —— 直接刷固件，不用装任何编译器

**最新固件**是
[`prebuilt/ws2812_stc8g1k08a_35mhz_brightness-rainbow.hex`](prebuilt/ws2812_stc8g1k08a_35mhz_brightness-rainbow.hex)。
这是**带亮度调节**（长按）的版本，同时支持颜色切换和**断电记忆**。
它由本仓库源码编译而来，且可逐字节复现。刷写步骤（纯文字版，不看图也能操作）：

**需要准备：** 一个 CH340 类 USB 转 TTL 串口模块（淘宝几块钱）和一台
Windows 电脑（macOS/Linux 方案见下）。

**步骤：**

1. **接 4 根线**（模块 ↔ 控制板）：
   - 模块 `TXD` → 单片机 `P3.0`（RxD）
   - 模块 `RXD` → 单片机 `P3.1`（TxD）
   - 模块 `GND` → `GND`
   - 模块 `5V` → `VCC`（让板子从模块取电，方便断电重上电）
2. **下载 STC-ISP**（免费官方软件）：
   <https://www.stcmicro.com/rjxz.html>，解压后直接运行
   `stc-isp-xxx.exe`，无需安装。
3. 在 STC-ISP 里"芯片型号"选 `STC8G1K08A-8PIN`，"串口号"选 CH340 对应的
   COM 口（插上模块就会出现；没有就先装 CH340 驱动）。
4. 点"打开程序文件"，选 `ws2812_stc8g1k08a_35mhz_brightness-rainbow.hex`。
5. **"输入用户程序运行时的IRC频率"必须填 `35.000` MHz** —— 固件里所有
   WS2812 时序都由这个频率推算，填错灯就不亮或乱闪。其余选项保持默认即可
   （完整的已验证设置截图见
   [docs/images/stc-isp-settings.png](docs/images/stc-isp-settings.png)）。
6. 点"下载/编程"，**然后再给板子重新上电**（拔掉再插上电源）。STC 芯片只在
   冷启动时进入烧录模式，所以顺序是：先点下载，后上电。
7. 等待"操作成功"。完成 —— 短按换颜色，长按调亮度。

**macOS / Linux：** 用开源的
[`stcgal`](https://github.com/grigorig/stcgal) 或
[`stc8prog`](https://github.com/IOsetting/stc8prog) 替代 STC-ISP，例如
`stcgal -P stc8d -t 35000 prebuilt/ws2812_stc8g1k08a_35mhz_brightness-rainbow.hex`
（`-t 35000` 即把 IRC 调到 35 MHz，规则同上）。

**排障：** 下载一直没反应时——先对调 TXD/RXD（最常见的错），确认 CH340
驱动装好了，并确认是"先点下载、后上电"。

> 注意：按截图的设置，"下次下载用户程序时擦除用户EEPROM区"是勾选的，
> 烧录会把记忆的颜色/亮度重置为默认值；想保留就取消该勾选。

## 功能

- **单键交互**
  - **短按** —— 循环切换 7 色：红 → 绿 → 蓝 → 品红 → 黄 → 青 → 白
    （松手时触发，与长按互不干扰）。
  - **长按**（≥ 500 ms）—— 按住期间亮度平滑渐升，越过最大值回绕到最小值；
    松手即停在当前亮度。
- **断电记忆** —— 颜色与亮度在松手时写入片内 EEPROM（IAP），上电自动恢复；
  用魔术字节校验数据有效性，只在松手时写入以减少擦写损耗。
- **统一亮度模型** —— 颜色以 R/G/B 开关标志存储，所有点亮通道共用同一个
  8 位亮度值，各颜色观感一致；亮度下限（`MIN_BRIGHT = 8`）保证灯带永远不会
  "看起来像坏了"，上限（`MAX_BRIGHT`）可用来限流。
- **可靠按键处理** —— 3 × 5 ms 消抖，短按/长按干净区分，渐变每 15 ms 走一步。
- **默认驱动 50 颗灯**（`LedNum` 可改），GRB 顺序。
- **双工具链，行为一致**
  - Keil C51（附 µVision 工程）—— 原始版本。
  - SDCC（免费开源，macOS/Linux/Windows 均可用）——
    `firmware/sdcc/build.sh`。SDCC 构建**可复现**：从本仓库重新编译得到的
    hex 与发布件逐字节一致。
- **极小体积** —— SDCC 版仅占 968 字节 Flash（共 8 KB）、150 字节 XRAM；
  Keil 版约 2–3 KB（多链接了一个浮点 HSV→RGB 函数，留作彩虹/渐变扩展钩子）。
- **附预编译、可直接烧录的 hex**（[prebuilt/](prebuilt/)，见"快速开始"）。

## 硬件

| 引脚 (SOP8) | 信号 | 作用 |
|------------|------|------|
| P3.2 | KEY | 按键对地（低电平按下，内部弱上拉，无需外接电阻） |
| P3.3 | DIN | WS2812 数据输入 |
| P3.0 / P3.1 | RxD / TxD | 串口，仅用于 CH340 类 USB 转串口烧录 |
| VCC / GND | 电源 | 5 V |

```
        5V ──────────────┬──────────────────────┐
                         │                      │
                   ┌─────┴─────┐    ~300Ω   ┌───┴────────┐
        KEY ──┐    │ STC8G1K08A│ P3.3 ─/\/\─┤ DIN  WS2812│ ⇒⇒⇒ (50 灯)
      (对地)  └────┤ P3.2      │            │      灯带  │
                   └─────┬─────┘            └───┬────────┘
                         │                      │
        GND ─────────────┴──────────────────────┘
```

**电源提示**

- MCU 用 5 V 供电，逻辑高电平才满足 WS2812 的数据阈值。
- 50 颗灯全白全亮可达 ~3 A —— 灯带要直接从电源取电，别走 MCU 板走线；
  默认亮度 26/255（约 10%）时电流很温和。
- 常规 WS2812 保护照做：数据线串 ~300 Ω 电阻、灯带电源并 ≥ 470 µF 电解电容。

## 原理 —— 时序难题怎么解

WS2812 在一根线上以 800 kbps 用**高电平脉宽**编码比特：`0` 约高
220–380 ns，`1` 约高 580–1000 ns，位周期 1.25 µs，低电平 > 50 µs 锁存。
8051 没有任何外设能生成这种波形，经典的 11.0592 MHz 下一条指令的粒度都嫌粗。

本固件的解法简单粗暴但可靠：

1. **内部 RC 振荡器超频到 35 MHz**（STC8G 出厂校准支持的档位，在 STC-ISP
   烧录界面里选，**不在代码里设**）。1T 内核下即 **28.6 ns 的时序分辨率**。
2. **关中断（`EA = 0`）+ 数周期逐位发送**，整帧期间任何中断都不可能拉长脉冲。
   SDCC 版用内联汇编做到完全确定的时序：

   | 参数 | 实测 (35 MHz) | WS2812B 规格窗口 |
   |------|--------------|------------------|
   | T0H | 12 clk ≈ 343 ns | 220–380 ns |
   | T1H | 24 clk ≈ 686 ns | 580–1000 ns |
   | T0L | 29 clk ≈ 829 ns | 580–1600 ns |
   | T1L | 17 clk ≈ 486 ns | 220–420 ns* |

   *不同批次 WS2812B 对 T1L 下限口径不一，两个构建都落在所有变体的宽容中段。

   值得学的一手：每个比特先用 `rlc a` 把 MSB 预取进进位标志**再**拉高引脚，
   高电平期间只有 `nop`，脉宽纯由 nop 数决定，与编译器代码生成无关。
3. 刷新 50 颗灯（150 字节 × 8 位 × 1.25 µs ≈ **1.5 ms**）全程关中断；
   主循环 5 ms 的节拍天然提供了 > 50 µs 的锁存间隙。

**必须遵守的约束**

- STC-ISP 里的 IRC 频率**必须是 35.000 MHz**，选别的值上面所有时序全部悄悄失效。
- 不要动两棵源码树里 `LedRefresh()` 的 `nop` 数量，也不要"优化"这个函数。
- 换数据引脚时，`sbit`/`__sbit` 定义和 `WS2812_Init()` 里的端口模式寄存器
  （`P3M0`/`P3M1`）要一起改。

## 仓库结构

```
.
├── README.md               ← 英文文档 / English
├── README.zh-CN.md         ← 本文件
├── PUBLISHING.md           ← 整理说明 + 发布到 GitHub 的步骤（英文）
├── LICENSE                 ← MIT（含捆绑厂商文件的说明）
├── docs/
│   └── images/stc-isp-settings.png   ← 验证过的 STC-ISP 烧录设置截图
├── firmware/
│   ├── keil/               ← 原始 Keil C51 工程（project/ + source/）
│   └── sdcc/               ← 免费工具链移植版（平铺源码 + build.sh）
└── prebuilt/               ← 最新的可直接烧录 hex（必须配 35 MHz IRC）
```

两棵源码树刻意不用 `#ifdef` 合并：`LedRefresh()` 是时序关键代码，按编译器
分别手工调校（Keil 用数好的 `_nop_()`，SDCC 用内联汇编），且都在真机上验证过。

## 从源码编译

**SDCC（免费，跨平台；prebuilt/ 里的固件就是它编译的）**

```sh
# macOS: brew install sdcc     Debian/Ubuntu: sudo apt install sdcc
cd firmware/sdcc
./build.sh                     # → WS2812_STC8G1K08A.hex
```

**Keil µVision（C51）**

1. Keil 器件库里没有 STC 芯片：打开 STC-ISP → "Keil仿真设置"标签 →
   "将STC MCU型号添加到Keil"，指向 Keil 安装目录（一次即可）。
2. 打开 `firmware/keil/project/WS2812_STC8G1K08A.uvproj`，器件选
   "STC8G1K08 Series"，编译（F7），hex 输出在 `firmware/keil/output/`。

自己编译出的 hex 烧录方法与"快速开始"完全相同 —— 同样的接线、同样的
STC-ISP 设置、同样必须 35.000 MHz。

## 可调参数

| 宏 | 文件 | 默认 | 含义 |
|----|------|------|------|
| `LedNum` | `ws2812.h` | 50 | 灯数。SDCC 树还需同步改 `LedRefresh()` 汇编里的 `#150`（= 灯数 × 3）。 |
| `COLOR_NUM` / `color_tab[]` | `ws2812.h` / `ws2812.c` | 7 色 | 调色板，两处保持同步。 |
| `DEFAULT_BRIGHT` | `ws2812.h` | 26（约 10%） | 首次上电亮度。 |
| `MIN_BRIGHT` / `MAX_BRIGHT` | `ws2812.h` | 8 / 255 | 渐变下限 / 上限（调低上限即限流）。 |
| `RAMP_STEP` | `ws2812.h` | 2 | 渐变每步增量。 |
| `DB_TICKS` | `key.h` | 3（约 15 ms） | 消抖窗口。 |
| `LONG_TICKS` | `key.h` | 100（约 500 ms） | 长按阈值。 |
| `RAMP_INTERVAL` | `key.h` | 3（约 15 ms） | 渐变步进周期。 |
| `EEPROM_ADDR` | `eeprom.h` | 0x0000 | 设置所在扇区（512 字节擦除粒度）。 |

## Keil 与 SDCC 版差异

| | Keil 树 | SDCC 树 |
|---|---|---|
| `LedRefresh()` | C 里数 `_nop_()` | 内联汇编，完全确定 |
| P3.3 驱动模式 | 准双向 | **推挽**（边沿更陡、驱动更强） |
| HSV→RGB 辅助函数 | 有（未使用的扩展钩子） | 无（保持 1 KB 以内） |
| Flash 占用 | 约 2–3 KB | 968 字节 |
| 本仓库是否附预编译 hex | 无（需自行编译） | **有** —— `prebuilt/ws2812_stc8g1k08a_35mhz_brightness-rainbow.hex` |

## 购买渠道（2026 年 7 月核对，价格仅供参考）

**成品直购 —— 我们自己的产品，见 [longxl.shop](https://longxl.shop)**

本固件家族就运行在我们的 FPV 成品灯具上，不想动烙铁可以直接买：

- **[DeepSpace × LongXL 联名 PHLUX 灯带](https://longxl.shop/products/deepspace-x-longxl-collaborative-phlux-led-strip)**
  —— 4 条可编址灯带（每条 12 灯）+ 单键控制器（3–5V 直供，或 2S–6S
  平衡头供电）：一键循环的 7 色与本仓库 `color_tab` 完全一致。
  单条 $3.99 起，整套 $14.99。
- **[HGLRC LED LongXL（13mm）](https://longxl.shop/products/hglrc-led-longxl-13mm)**
  —— 超轻 0.12g 的 13.3 × 4.3mm 可编址灯组，3.3–5.5V，SH1.0 接头，
  4 只装 $7.99，与上面的控制器配套使用。

成品相关问题请联系：**<longxinpeng@longxlshop.com>**。

**主控 STC8G1K08A**

- **立创商城**（szlcsc.com，编号 C915663，SOP8）约 ¥1–2/片；海外走
  LCSC 国际站约 US $0.27 @ 5 片起。保真首选，也可在嘉立创 SMT 贴片。
- **STC 官方淘宝直营店**（官网产品页 <https://www.stcmicro.com/stc/stc8g1k08a.html>
  的 "Buy Online" 链接直达）。
- AliExpress / 淘宝散件 5 片装约 ¥7–12；DIP8 直插件基本只在这类渠道有。
  Digi-Key / Mouser 不卖 STC。

**WS2812B 灯带**

- 国内：淘宝/京东搜"WS2812B 幻彩灯带 5V"，60 灯裸板约 ¥8–20/米。
- 国际：BTF-LIGHTING 官网 / Amazon 官方店（5 m 60 灯 IP30 约 US $20–25）/
  AliExpress 官方店（约 $1.2–2.8/米）。低于 $1/米的杂牌常见假 IC，慎买。

**烧录工具**

- CH340 USB 转串口模块：淘宝约 ¥3–10，AliExpress 约 $1–2。
- STC-ISP 软件：免费，<https://www.stcmicro.com/rjxz.html>。
- 可选：STC-USB Link1D 官方仿真/烧录器约 ¥40+（立创 C5328703）。

**整机 BOM 成本不到 ¥15**（主控 + 按键 + 阻容），灯带另计。

**第三方通用控制器**（对比参考）：SP002E 类三键控制器（¥3–15 / US $1–5）、
BTF SP621E 系列、射频遥控款、SP108E Wi-Fi 款。没有一款是真正单键的 ——
想要单键，见本节开头我们的 PHLUX 套装。

## 扩展方向

- 彩虹/渐变/流水效果 —— Keil 树里现成的 `HSVtoRGB()` 就是为此准备的。
- 双击开关灯；或把亮度渐变改成到顶反弹（乒乓）而非回绕（原作者注释里
  考虑过这个手感）。
- 伽马校正表，让亮度渐变在人眼里更线性。
- 剩余空脚（P5.4/P5.5）加第二个按键或光敏电阻。

## 支持与联系

- 发现 bug 或有疑问？欢迎提 GitHub Issue。
- 也可以直接发邮件联系我们：**<longxinpeng@longxlshop.com>** ——
  对固件或硬件的任何反馈都欢迎。

## 许可证

[MIT](LICENSE)。捆绑的 `STC8G.H`（STC 官方）与 `STARTUP.A51`（Keil 随附）
按其原厂商条款分发，见 LICENSE 文件末尾说明。
