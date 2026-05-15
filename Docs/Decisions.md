# 决策记录

## D001：使用干净项目

当前正式项目使用：

`G:\ue5_projects\rpg\rpg260514`

之前的 `G:\ue5_projects\rpg\my_project` 是试验目录，除非明确要求，否则不再编辑。

## D002：使用 Content 根级分类

不再创建 `Content/RPG` 包装目录。

这个项目是干净空白项目，所以游戏内容直接放在 `Content` 根级分类下，例如 `Characters`、`Combat`、`AI`、`Items`、`Quests`、`UI`。

## D003：先白盒

第一阶段使用简单几何体、占位材质和临时角色。

正式美术风格、渲染方案和资产选择，等第一个可玩循环完成后再决定。

## D004：第三人称探索

探索阶段使用第三人称自由视角。

这个方向更适合剧情、世界尺度、队友存在感和后续镜头演出。

## D005：第一版先半即时，但保持可切换

原型战斗先做半即时，因为它适合第三人称探索和队友自动行动。

但底层必须围绕单位、行动、技能和战斗遭遇来设计，让以后可以把行动调度替换成回合制。

## D006：数值数据驱动

HP、MP、ATK（攻击）、DEF（防御）、冷却、距离、奖励、任务需求等 RPG 数值不要写死在角色逻辑中。

第一阶段使用 DataAsset。以后数据量变大，再考虑 DataTable 或外部表格。

## D007：输入系统只使用 Enhanced Input

项目不混用旧版 `ActionMappings / AxisMappings` 和 UE5 的 Enhanced Input。

角色代码只绑定 `UInputAction`，玩家控制器负责添加 `UInputMappingContext`。这样后续可以按状态切换输入上下文，例如探索、战斗、UI、对话，而不需要在角色类里写大量按键判断。

当前代码已迁移到 Enhanced Input，输入资产已创建在 `Content/Input`：

- `IA_Move`
- `IA_Look`
- `IA_Jump`
- `IA_Interact`
- `IA_PrimaryAction`
- `IA_Sprint`
- `IMC_Exploration`

当前实现已收敛为：

- 原型阶段默认启用 `bForceRuntimeExplorationMapping`，优先使用代码生成的完整探索 MappingContext。
- 运行时映射包含 WASD、鼠标视角、手柄视角、滚轮缩放、跳跃、交互、基础行动和 Shift 闪避。
- `IMC_Exploration` 暂时保留为编辑器可视化和后续平台差异化入口，但不作为当前原型的唯一输入事实来源。
- 如果后续关闭 `bForceRuntimeExplorationMapping`，`IMC_Exploration` 必须至少包含：`IA_Move` 的 WASD / Gamepad Left2D，`IA_Look` 的 MouseX / MouseY / Gamepad Right2D，`IA_Zoom` 的 MouseWheelAxis，`IA_Jump` 的 Space / Gamepad Bottom，`IA_Interact` 的 E，`IA_PrimaryAction` 的 LeftMouseButton，`IA_Sprint` 的 LeftShift。

这样可以避免 `IMC_Exploration` 资产里的按键或 Modifier 配置不完整时，出现“按键没反应”或“鼠标不能转镜头”的问题。

## D010：手感参数暴露到蓝图

需要频繁调试的手感参数不要写成 C++ 局部常量。

当前 Dash 参数已通过 `UPROPERTY(EditAnywhere, BlueprintReadWrite)` 暴露到 `BP_PlayerCharacter`：

- `DashStrength`
- `DashDuration`
- `DashCooldown`
- `bDashUseInputDirection`
- `bDashUseRootMotionAnimation`
- `bDashApplyMovementImpulse`
- `DashAnimation`
- `DashFallbackAnimation`
- `DashAnimationBlendIn`
- `DashAnimationBlendOut`
- `DashAnimationPlayRate`
- `DashAnimationSlotName`

`DashAnimation` 可以先填普通 `AnimSequence`，角色代码会动态转成 Montage 播放；后续如果需要动画通知、无敌帧或位移曲线，也可以直接替换成正式 `AnimMontage`。动态 Montage 的 Slot 名通过 `DashAnimationSlotName` 暴露，当前默认使用官方 `ABP_Unarmed` 可用的 `DefaultSlot`。

默认闪避的实际位移由 `LaunchCharacter` 控制，所以自动选择脚本默认避开 `/RootMotion/` 动画。若蓝图里误填了 RootMotion 闪避动画，运行时会改播 `DashFallbackAnimation`，避免 Mesh 相对胶囊跑到屏幕边缘后在动画结束时回弹。

如果要使用 `A_Roll_IdleFwd` 这类自带位移的 Roll 动画，可以在 `BP_PlayerCharacter` 中把 `DashAnimation` 指向该 RootMotion 动画，并勾选 `bDashUseRootMotionAnimation`。当前原型阶段仍默认保留 `bDashApplyMovementImpulse = true`，即播放 RootMotion 动画的同时叠加一次短促代码位移，确保动画 Slot 或 RootMotion 配置出问题时，玩家也能从表现上看到闪避发生。

闪避现在会在屏幕上显示运行时 Debug 信息，包括速度、MovementMode、Dash 状态、最近一次 Dash 输入时间、当前动画和位移模式。该功能由 `bShowMovementDebug` 控制，方便后续调试手感。

后续相机、移动、战斗手感也遵循这个原则：底层规则写在 C++，调参入口暴露给蓝图或 DataAsset。

## D011：会话记忆和文件删除规则

每次开发会话结束前，都要更新本次会话自己的 session memory，记录项目路径、当前功能状态、关键决策、验证方式和后续注意事项。一个会话只维护自己的 memory 文件，不要修改其它会话的 memory 文件。

不要在未获得用户明确同意的情况下删除任何文件。即使文件看起来像临时文件、自动生成文件、记忆文件或无关文件，也必须先说明原因并等待用户确认。

## D008：免费资源使用原则

原型阶段可以使用 UE 自带免费资源和 Epic 免费资源。

官方或外部资产包尽量保留其内部依赖期望的路径，例如当前 Mannequins 资源使用 `Characters/Mannequins`。这样可以减少材质、动画、蓝图等资源引用断裂的风险。

项目自有资源继续放入 `Characters`、`Combat`、`AI`、`UI` 等分类目录。等原型稳定后，再决定是否把外部资源整理、替换或重做。

## D009：原型阶段复用官方 ABP

当前玩家蓝图使用 UE 免费资源中的 `ABP_Unarmed` 驱动移动动画。

不要在 C++ 里强制切换 Mesh 的 `AnimationMode`，避免覆盖蓝图里的动画蓝图设置。等原型需要更细的跳跃、攻击、受击、武器状态时，再制作项目自己的 `ABP_PlayerBase`。
