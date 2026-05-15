# Character Controller 角色控制系统

## 1. 系统定位

Character Controller 是玩家角色在探索阶段的基础控制系统。

它负责把玩家输入转化为角色移动、镜头控制、跳跃、闪避、交互入口和基础行动入口。

本系统不负责：

- 具体 NPC 交互内容。
- 战斗伤害结算。
- 技能数据。
- 任务状态。
- 正式动画状态机。

核心原则：

> 角色控制器只解决“玩家如何舒服地控制角色”，不把战斗、任务、剧情逻辑堆进玩家角色类。

## 2. 当前目标

原型 001 阶段需要优先保证：

1. 玩家可以在第三人称视角下稳定移动。
2. 鼠标控制自由镜头，滚轮控制镜头距离。
3. Shift 可以触发一次性闪避/翻滚，而不是持续冲刺。
4. 闪避必须在表现上可见，不能只依赖动画资产是否正确。
5. 屏幕上能显示速度和状态 Debug 文本，方便定位手感问题。
6. 输入系统统一使用 Enhanced Input，不混用旧版 `AxisMapping / ActionMapping`。

## 3. 当前代码结构

当前角色控制相关代码位于：

```text
Source/rpg260514/Character/RPGPlayerCharacter.h
Source/rpg260514/Character/RPGPlayerCharacter.cpp
Source/rpg260514/Core/RPGPlayerController.h
Source/rpg260514/Core/RPGPlayerController.cpp
```

职责划分：

- `ARPGPlayerCharacter`
  - 处理移动、镜头、缩放、跳跃绑定、闪避、交互入口、基础行动入口。
  - 暴露镜头和闪避手感参数给蓝图调试。
  - 显示运行时 Debug 文本。
- `ARPGPlayerController`
  - 添加探索输入上下文。
  - 管理 Enhanced Input 的 MappingContext。
  - 在输入资产缺失关键映射时提供运行时兜底。
- `BP_PlayerCharacter`
  - 配置 Mesh、动画蓝图、手感参数、动画资产。
- `BP_RPGGameMode`
  - 指定默认 Pawn 和 PlayerController。

## 4. 输入设计

项目只使用 Enhanced Input。

当前输入语义：

```text
IA_Move          WASD / Gamepad Left2D
IA_Look          MouseX / MouseY / Gamepad Right2D
IA_Zoom          MouseWheelAxis
IA_Jump          Space / Gamepad Bottom
IA_Interact      E
IA_PrimaryAction LeftMouseButton
IA_Sprint        LeftShift
```

注意：

- `IA_Sprint` 文件名暂时沿用旧命名，但玩法语义已经是 Dash / Dodge。
- 后续可以在编辑器里重命名为 `IA_Dash` 或 `IA_Dodge`，但需要同步修改 C++ 和输入脚本。
- 角色代码只绑定 InputAction，不直接写死按键。
- 默认优先使用 `IMC_Exploration`，保证编辑器里的按键调整、平台差异化映射和重绑测试能生效。
- `bForceRuntimeExplorationMapping` 仅作为原型期应急开关，默认关闭；打开后会绕过 `IMC_Exploration`。
- 当 `IMC_Exploration` 缺失关键映射且 `bUseRuntimeMappingFallback=true` 时，运行时会追加一套兜底探索映射。
- `IMC_Exploration` 的 WASD 和 MouseX/MouseY 需要与 runtime fallback 使用同一套 modifier；`Scripts/update_exploration_input_mapping.py` 是当前同步脚本。
- Play 开始时 `RPGPlayerController` 会进入 `GameOnly` 输入模式，并隐藏鼠标光标，避免鼠标被 UI/光标状态截走。

## 5. 移动设计

移动使用标准 UE `CharacterMovementComponent`。

当前基础参数：

```text
WalkSpeed = 500
JumpZVelocity = 500
AirControl = 0.35
RotationRate = 540 deg/s
BrakingDecelerationWalking = 2000
BrakingDecelerationFalling = 1500
```

移动方向按镜头水平朝向计算：

```text
W/S -> 摄像机水平 Forward
A/D -> 摄像机水平 Right
```

设计意图：

- 第三人称探索时，玩家的移动方向跟随镜头理解。
- 角色身体朝移动方向转向，不强行跟随控制器 Yaw。
- 后续如果加入锁定目标，可以在战斗状态下替换转向规则。

## 6. 镜头设计

当前镜头结构：

```text
Character
  CapsuleComponent
  SpringArmComponent CameraBoom
    CameraComponent FollowCamera
```

镜头参数暴露到 `BP_PlayerCharacter`：

```text
CameraDistance
MinCameraDistance
MaxCameraDistance
ZoomStep
MouseLookSensitivity
GamepadLookSensitivity
```

当前行为：

- 鼠标移动控制水平和俯仰视角。
- 鼠标滚轮只控制镜头距离。
- 镜头缩放不再和鼠标移动混在一起。

后续可扩展：

- 镜头碰撞调优。
- 肩后视角偏移。
- 锁定目标时的镜头偏置。
- 对话、调查、战斗状态下的临时镜头模式。

## 7. 闪避设计

Shift 当前语义是一次性 Dash / Dodge，不是持续 Sprint。

当前参数暴露到 `BP_PlayerCharacter`：

```text
DashStrength
DashDuration
DashCooldown
bDashUseInputDirection
bDashUseRootMotionAnimation
bDashApplyMovementImpulse
DashAnimation
DashFallbackAnimation
DashAnimationBlendIn
DashAnimationBlendOut
DashAnimationPlayRate
DashAnimationSlotName
```

### 7.1 闪避方向

优先使用最近的移动输入方向。

如果没有移动输入，或 `bDashUseInputDirection = false`，则使用当前镜头水平 Forward。

这样可以支持：

- 按方向键 + Shift：朝输入方向闪避。
- 原地 Shift：朝镜头前方闪避。

### 7.2 位移方案

当前使用“动画 + 代码冲量”的混合方案。

流程：

```text
1. 检查冷却。
2. 计算 DashDirection。
3. 播放 DashAnimation。
4. 如果 bDashApplyMovementImpulse = true，调用 LaunchCharacter 产生明确位移。
5. DashDuration 后清掉水平速度。
6. DashCooldown 后恢复可闪避状态。
```

原因：

- RootMotion 动画如果 Slot、ABP、RootMotion 提取任意一环出问题，玩家可能看起来完全没动。
- 原型阶段需要优先保证操作反馈明确。
- 动画可以先负责“看起来像翻滚”，代码冲量负责“真的移动了”。

### 7.3 RootMotion 模式

`bDashUseRootMotionAnimation = true` 时：

- 播放前会把角色朝向本次闪避方向。
- 会先停止当前普通移动速度。
- 仍会叠加一次代码冲量，避免动画配置失败导致无表现。

这不是最终战斗手感的唯一方案，而是原型阶段更稳的调试方案。

后续如果 RootMotion 动画链路稳定，可以考虑：

- 关闭 `bDashApplyMovementImpulse`，只用 RootMotion 位移。
- 使用 In-Place 动画，完全由 `LaunchCharacter` 控制距离。
- 增加 RootMotion 距离缩放参数。
- 改成正式 `AnimMontage`，加入无敌帧和动画通知。

## 8. Debug 显示

当前 `ARPGPlayerCharacter` 每帧通过 `GEngine->AddOnScreenDebugMessage` 显示运行时 Debug 文本。

显示内容：

```text
Speed XY
Velocity X/Y/Z
Movement Mode
Dash State
Last Dash Input Time
Last Dash Event
Dash Mode
Impulse On/Off
DashAnimation
```

用途：

- 判断 Shift 是否真正触发了 Dash。
- 判断角色速度是否在 Dash 时增加。
- 判断当前是否卡在 Cooldown。
- 判断当前使用 RootMotion 还是普通位移。
- 判断是否播放了预期动画。

可调开关：

```text
bShowMovementDebug
```

位置：

```text
BP_PlayerCharacter -> Debug -> Show Movement Debug
```

后续建议：

- 增加快捷键开关 Debug。
- 做一个 UMG 调试面板，集中显示镜头、移动、闪避和战斗状态。
- Debug 功能仅用于开发，不进入正式 UI。

## 9. 状态划分

角色控制建议逐步拆成状态，而不是把所有输入永久混在一起。

建议状态：

```text
Exploration  探索
Combat       战斗
Dialogue     对话
Interaction 交互调查
Menu         菜单
Cutscene     演出
```

当前只实现 `Exploration`。

后续状态切换原则：

- PlayerController 负责添加/移除 MappingContext。
- Character 负责执行动作语义。
- 具体系统负责处理业务逻辑。

例如：

```text
探索状态：左键 = PrimaryAction 占位
战斗状态：左键 = 普通攻击
对话状态：左键 = 推进文本
菜单状态：左键 = UI 点击
```

同一个按键可以在不同状态下做不同事，但不要在角色类里堆大量 if 判断。

## 10. 动画设计边界

当前玩家 Mesh 使用 Manny，动画蓝图使用官方 `ABP_Unarmed`。

当前规则：

- 不在 C++ 里强制切换 Mesh 的 AnimationMode。
- 不绕过 ABP 使用 `AnimationSingleNode` 播放单个动画。
- 闪避动画通过动态 Montage 播放。
- 默认 Slot 使用 `DefaultSlot`。

后续建议：

- 原型稳定后创建项目自己的 `ABP_PlayerBase`。
- 把移动、跳跃、闪避、攻击、受击分层管理。
- 闪避、攻击等动作改为正式 `AnimMontage`。
- 使用 AnimNotify 标记无敌帧、位移窗口、打断窗口、声音和特效。

## 11. 设计风险

### 风险一：角色类过度膨胀

表现：

- 角色类同时处理移动、战斗、交互、任务、UI。

解决：

- 角色类只保留输入入口和角色基础能力。
- 伤害结算放到 CombatComponent。
- 可交互查询放到 InteractionComponent。
- 任务逻辑放到 Quest 系统。

### 风险二：动画和位移互相打架

表现：

- RootMotion 推动角色，代码也推动角色，导致距离过长。
- Mesh 和 Capsule 不一致，动画结束时回弹。

解决：

- 原型阶段保留 Debug 速度显示。
- 明确当前使用哪种位移源。
- 正式化时选择 RootMotion 或 In-Place + 代码位移中的一种作为主方案。

### 风险三：输入资产和代码双事实来源

表现：

- 资产里有一套映射，代码运行时又创建一套映射，导致重复触发或行为不一致。

解决：

- 优先使用 `IMC_Exploration`。
- 运行时只补关键兜底映射。
- 后续用脚本验证输入资产完整性。

## 12. 最小可行版本

原型 001 的 Character Controller MVP：

```text
[x] WASD 移动
[x] 鼠标自由镜头
[x] 滚轮缩放
[x] Space 跳跃
[x] Shift 闪避
[x] 闪避速度和状态 Debug
[x] Enhanced Input 输入链路
[x] Play 模式下基础移动、镜头和 Shift 闪避可用
[ ] Debug 显示快捷开关
[ ] 白盒地图中验证坡道、障碍、转角和镜头碰撞
```

下一阶段优先级：

1. 在白盒地图中验证移动和镜头手感。
2. 调整 DashStrength / DashDuration / DashCooldown。
3. 决定闪避最终使用 RootMotion 还是 In-Place + 代码位移。
4. 增加交互组件，让 `Interact()` 不只是占位。
5. 增加 CombatComponent，让 `PrimaryAction()` 转发基础攻击。
6. 在资产层正式把 `IA_Sprint` 重命名为 `IA_Dash` 或 `IA_Dodge`，同步更新 C++、脚本和蓝图引用。
