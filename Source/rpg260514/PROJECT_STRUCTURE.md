# Source 目录结构

C++ 系统按玩法职责分组。只有当功能确实需要代码时再添加类，不提前创建没有行为的空抽象。

- `Core`：GameInstance、SaveGame、项目通用类型、GameMode、PlayerController
- `Character`：玩家、NPC、敌人的角色逻辑
- `Combat`：攻击、伤害、目标选择、状态效果
- `AI`：Controller、感知、StateTree 辅助逻辑
- `Inventory`：物品实例、容器、装备
- `Quest`：任务状态、任务目标、对话挂接
- `Interaction`：可交互组件和交互目标选择
- `UI`：HUD 控制器和面向 Widget 的类
- `Data`：DataAsset 类、DataTable 行结构
