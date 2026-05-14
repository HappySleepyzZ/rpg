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
- `IMC_Exploration`

## D008：免费资源使用原则

原型阶段可以使用 UE 自带免费资源和 Epic 免费资源。

官方或外部资产包尽量保留其内部依赖期望的路径，例如当前 Mannequins 资源使用 `Characters/Mannequins`。这样可以减少材质、动画、蓝图等资源引用断裂的风险。

项目自有资源继续放入 `Characters`、`Combat`、`AI`、`UI` 等分类目录。等原型稳定后，再决定是否把外部资源整理、替换或重做。

## D009：原型阶段使用最小动画驱动

官方 `ABP_Unarmed` 不是通用即插即用动画蓝图，直接挂到当前 `ARPGPlayerCharacter` 上不会可靠驱动移动动画。

原型 001 先使用 C++ 根据角色水平速度驱动 `BS_Idle_Walk_Run`，让玩家可以看到 Idle/Walk/Run。这个方案只用于原型阶段，后续需要制作正式 `ABP_PlayerBase`，再把跳跃、攻击、受击等状态接入动画蓝图。
