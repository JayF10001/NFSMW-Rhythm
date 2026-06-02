# MWRhythm

MWRhythm is a fork of CarbonRhythm, a rhythm "minigame" mod originally built for Need for Speed: Carbon.
This is my version just to commit changes to the Main Fork.
This fork ports the plugin from Need for Speed:Carbon, to Need for Speed: Most Wanted (NFSMW).

---

## Features

- 6-lane rhythm input system
- Car animations
  - They change depending on BPM
  - Every key start the Animation. Or Extends the previous one.
- Beat-based System for note scrolling
  - Has dynamic Scaling depending on Screen Resolution
  - Scrolling speed changes depending on BPM
- Judgement system (Perfect / Good / Bad / Miss)
- Combo and multiplier system
- Hit Sounds!
- Accuracy calculation
- Custom beatmap support
  - Support for changingBPM of a song, while it is still playing
- Visual overlay with animated UI elements
- Simple AutoPlay System [not perfect but it gets the job done], by default on F1

---

## Build

See [BUILD.md](BUILD.md) for Visual Studio/MSBuild setup, required
`ThirdParty/` dependencies, and troubleshooting notes.

---

## External Dependencies

Local build dependencies live under `ThirdParty/`. This directory is ignored by
Git; see [BUILD.md](BUILD.md) for the expected layout.

---

## Credits

- **giovannosaur** - Original author of CarbonRhythm

---

## License

MinHook is licensed under the BSD 2-clause license.
nlohmann/json is licensed under the MIT license.
BASS is free for non-commercial use.

---

## Disclaimer

This project is not affiliated with or endorsed by EA Games.
Need for Speed is a property of Electronic Arts.

---

# MWRhythm（中文版）

MWRhythm 是 CarbonRhythm 的一个 fork。CarbonRhythm 是为《极品飞车：生死卡本》（Need for Speed: Carbon）制作的节奏“小游戏”模组。
本 fork 将该插件从 NFS: Carbon 移植到《极品飞车：最高通缉》（Need for Speed: Most Wanted / NFSMW）。

---

## 功能

- 6 键道节奏输入系统
- 车辆动画
- 跟随节拍滚动的音符
- 判定系统（Perfect / Good / Bad / Miss）
- 连击与倍率系统
- 准确率计算
- 自定义谱面支持
- 带动画 UI 元素的视觉叠加层

---

## 构建依赖

构建需要以下依赖：

- Microsoft Visual Studio
- DirectX 9 SDK（June 2010）
- BASS 音频库
- MinHook（已包含：头文件 + lib）
- nlohmann/json（已包含：单头文件）

---

## 外部依赖

`/External` 中包含：

- MinHook（头文件 + x86 lib）
- json.hpp

---

## 致谢

- **giovannosaur** - CarbonRhythm 原作者

---

## 许可

MinHook 使用 BSD 2-clause 许可证。
nlohmann/json 使用 MIT 许可证。
BASS 可免费用于非商业用途。

---

## 免责声明

本项目与 EA Games 无关，也未获得其认可或背书。
Need for Speed 为 Electronic Arts 的资产。
