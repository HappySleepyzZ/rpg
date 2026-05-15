# 260515 RootMotion 闪避会话记忆

## 项目和用户偏好

- 正式项目路径：`G:\ue5_projects\rpg\rpg260514`
- UE 版本：5.7
- UE 安装路径：`G:\Program Files\Epic Games\UE_5.7`
- 用户偏好：中文解释，代码里可以多写中文注释，架构要可维护，调试参数尽量暴露到 UE 编辑器，不要写死。
- 重要协作规则：一个会话只维护自己的 session memory，不要修改其它会话的 memory 文件。
- 重要协作规则：不要在未获得用户明确同意的情况下删除任何文件。

## 当前项目状态

- 输入系统使用 Enhanced Input，不混用旧版 `AxisMapping / ActionMapping`。
- 玩家蓝图：`Content/Characters/Player/BP_PlayerCharacter.uasset`
- 玩家角色 C++：`Source/rpg260514/Character/RPGPlayerCharacter.*`
- 玩家可 WASD 移动、鼠标转镜头、滚轮缩放、Shift 闪避/翻滚。
- 玩家 Mesh 使用 Manny，动画蓝图使用官方 `ABP_Unarmed`。

## 本会话完成内容

- 导入并提交 FreeSampleAnimationSet 相关资源。
- 新增 Git LFS 规则 `.gitattributes`，跟踪 UE 二进制资产。
- 新增 `IA_Sprint` 和 `IA_Zoom` 输入资产。
- 移除早期单节点 BlendSpace 驱动，保留蓝图里的官方 `ABP_Unarmed`。
- 在 `ARPGPlayerCharacter` 中加入可调闪避参数：
  - `DashStrength`
  - `DashDuration`
  - `DashCooldown`
  - `bDashUseInputDirection`
  - `bDashUseRootMotionAnimation`
  - `DashAnimation`
  - `DashFallbackAnimation`
  - `DashAnimationBlendIn`
  - `DashAnimationBlendOut`
  - `DashAnimationPlayRate`
  - `DashAnimationSlotName`
- 闪避支持两种模式：
  - 普通模式：`bDashUseRootMotionAnimation = false`，使用 `LaunchCharacter` 控制位移。
  - RootMotion 模式：`bDashUseRootMotionAnimation = true`，不调用 `LaunchCharacter`，播放前把角色朝向闪避方向，让动画 RootMotion 控制位移。

## 当前闪避配置

- 当前 `BP_PlayerCharacter` 使用 RootMotion Roll 闪避。
- 当前 `DashAnimation` 指向：
  `/Game/FreeSampleAnimationSet/Animations/DashDodgeRollSet/Mannequin/RootMotion/Roll/A_Roll_IdleFwd.A_Roll_IdleFwd`
- 已通过 UE Python 把 `A_Roll_IdleFwd.enable_root_motion` 从 `False` 改为 `True`。
- 这一步很关键：如果动画资产没有启用 Root Motion 提取，Mesh 会相对胶囊滚出去，动画结束时看起来像镜头或角色瞬间回弹。
- 当前用于配置 Roll 闪避的脚本：
  `Scripts/configure_root_motion_dash_roll.py`
- 当前用于自动选择普通闪避动画的脚本：
  `Scripts/set_dash_animation_from_imported_assets.py`

## 验证结果

- UE 5.7 构建命令已通过：
  `Build.bat rpg260514Editor Win64 Development -Project=G:\ue5_projects\rpg\rpg260514\rpg260514.uproject -WaitMutex -FromMsBuild`
- 修复后已启动项目验证，`UnrealEditor.exe` 成功运行。
- 最新启动日志中没有 `Missing Modules`、`could not be compiled` 或项目模块加载失败。

## 后续注意事项

- 如果用户要调整翻滚动作手感，优先在 `BP_PlayerCharacter -> Movement|Dash` 中调：
  - `DashAnimationPlayRate`
  - `DashCooldown`
  - `DashAnimationBlendIn`
  - `DashAnimationBlendOut`
- RootMotion 模式下，`DashStrength` 和 `DashDuration` 基本不控制位移距离；位移主要来自动画根运动。
- 如果以后要改 RootMotion 距离，考虑新增可调的 RootMotion 缩放方案，或制作 In-Place 版本动画再用 `LaunchCharacter` 控制距离。
- 以后每次改完 UE bug，按用户要求要帮忙启动项目验证。
