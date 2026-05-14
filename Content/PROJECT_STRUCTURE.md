# Content 目录结构

这是一个干净的 RPG 项目。游戏自有内容直接放在 `Content` 根级分类下，不额外套 `RPG` 包装目录。

- `Maps`：可玩地图、测试地图、世界分区地图
- `Characters`：玩家、NPC、敌人的模型、动画蓝图、蒙太奇
- `Combat`：武器、受击表现、伤害资源、战斗调参资源
- `AI`：StateTree、行为树、EQS、黑板、AI 数据
- `Items`：装备、消耗品、掉落物、物品图标
- `Inventory`：背包 UI 资源和物品展示资源
- `Quests`：任务数据、对话数据、目标资源
- `UI`：HUD、菜单、控件蓝图
- `Data`：DataAsset、DataTable、曲线、调参数据
- `VFX`：Niagara、粒子、命中特效
- `Audio`：音乐、音效、语音、SoundCue
- `Materials`：共享主材质、材质实例、材质函数
- `Environment`：道具、关卡美术、植被、灯光资源
- `StarterContent`：UE 自带免费占位资源，保留原始目录名
- `Characters/Mannequins`：UE 自带免费人形角色资源，保留资源内部依赖期望的路径以减少引用断裂
