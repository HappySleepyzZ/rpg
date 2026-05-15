# 通用交互系统设计

## 目标

原型阶段只解决探索态的通用交互入口：

- 玩家靠近可交互物时显示提示。
- 玩家按 `E` 时调用当前目标的交互行为。
- 玩家角色不直接写 NPC、物品、任务或战斗逻辑。
- 不实现背包、任务、战斗，只保留这些系统后续接入交互的入口。

## 当前实现

代码位于 `Source/rpg260514/Interaction`。

- `UInteractableInterface`：可交互对象实现的统一接口，等价于 Blueprint Interface。
- `UInteractionDetectorComponent`：玩家身上的交互探测组件，继承 `USphereComponent`。
- `ARPGInteractableActor`：可直接派生的原型交互 Actor，自带交互范围、提示文本和蓝图事件。
- `ARPGPlayerCharacter`：创建 `InteractionDetector`，并在 `Interact()` 输入回调里调用 `TryInteract()`。

### 接口

`UInteractableInterface` 暴露 3 个 Blueprint Native Event：

- `CanInteract(AActor* Interactor)`：判断当前交互者是否允许交互。
- `GetInteractionPrompt(AActor* Interactor)`：返回屏幕提示文本。
- `Interact(AActor* Interactor)`：执行实际交互。

蓝图 Actor 可以直接实现该接口；C++ Actor 也可以实现对应 `_Implementation` 函数。

`ARPGInteractableActor` 已实现该接口，适合作为白盒阶段的基础类。派生蓝图后设置 `InteractionPrompt` 和 `VisualMesh`，需要响应交互时实现 `OnInteracted` 即可。若勾选 `bDisableAfterInteraction`，该对象交互一次后会自动变为不可交互。

当前测试资产计划为 `/Game/Interaction/BP_TestOneShotInteractable`，地图中实例标签为 `Test One Shot Interactable`。它的行为是：

- 靠近时提示 `Press E to test`。
- 第一次按 `E` 后输出一次 Debug 交互消息。
- 交互完成后 `bCanInteract=false`，下一帧不会再被选为当前可交互目标，不再显示提示，也不会再次响应 `E`。

当前测试物放置在唯一默认地图 `/Game/Maps/Prototype001` 中。`Prototype001` 使用 UE OpenWorld/Untitled 模板作为底板，不再另建单独测试地图。测试物被放在玩家起点附近，使用较大的亮色立方体表现，方便进入地图后直接定位。

### 目标选择

`UInteractionDetectorComponent` 使用半径检测维护候选列表：

- 默认半径：`220`。
- 候选列表只记录实现 `UInteractableInterface` 的 Actor。
- `CanInteract` 只在选择当前目标时判断，因此对象状态可以在重叠期间从不可交互切换为可交互。
- 多个目标同时存在时，按玩家到候选 Actor 的实体/可见组件包围盒最近点的距离排序；`QueryOnly` 触发范围不参与排序，避免多个候选都因进入触发球而距离变成 0。没有有效实体/可见包围盒时才回退到 Actor 原点。
- 当前提示先用 `GEngine->AddOnScreenDebugMessage` 显示，后续 HUD/UMG 就绪后替换显示层即可。

`CanInteract` 会在选目标和按键交互时被查询，必须保持为便宜、无副作用的纯状态判断。后续如果它需要读取任务、背包、剧情条件，应优先读取已有缓存状态，不在该函数里做昂贵搜索或触发业务流程。

### 输入

项目已有 `IA_Interact`，`ARPGPlayerCharacter::SetupPlayerInputComponent` 已绑定到 `Interact()`。

运行链路：

1. 玩家进入目标半径。
2. `InteractionDetector` 收集目标并显示 `GetInteractionPrompt`。
3. 玩家按 `E`。
4. `ARPGPlayerCharacter::Interact()` 调用 `InteractionDetector->TryInteract()`。
5. 组件对当前最近目标执行 `IInteractableInterface::Execute_Interact`。

## 蓝图接入方式

创建可交互蓝图时：

1. 在 Class Settings 中添加 `InteractableInterface`。
2. 实现 `CanInteract`，原型期通常直接返回 true。
3. 实现 `GetInteractionPrompt`，例如返回 `Press E to talk`。
4. 实现 `Interact`，只写该对象自己的行为。

交互行为的归属原则：

- NPC 对话入口写在 NPC 自己或 NPC 的对话组件里。
- 门、机关、采集点写在对应 Actor 自己或专用组件里。
- 任务、背包、战斗以后通过各自系统接收交互事件，不写进玩家角色。

## 后续替换点

- 提示显示：从屏幕 Debug 迁移到 HUD/UMG。
- 目标排序：从最近目标升级为视线优先、角度优先或锁定目标。
- 权限判断：`CanInteract` 可接入状态、阵营、任务条件或剧情条件，但必须保持纯查询。
- 表现层：`Interact` 内可触发动画、音效、镜头和 UI，但不要把表现层反写进探测组件。

## 非目标

本次不实现：

- 背包拾取和物品堆叠。
- 任务发布、追踪和完成。
- 战斗进入、攻击或伤害。
- 正式交互 UI。
