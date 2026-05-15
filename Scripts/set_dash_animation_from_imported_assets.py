import unreal


PLAYER_BLUEPRINT_PATH = "/Game/Characters/Player/BP_PlayerCharacter"
DASH_SLOT_NAME = "DefaultSlot"
PREFER_ROOT_MOTION_DASH_ANIMATION = False

# 免费动画包导入后常见的命名会包含这些词。分数越高，越优先被选为闪避动画。
PREFERRED_KEYWORDS = {
    "dodge": 100,
    "evade": 95,
    "roll": 90,
    "inplace": 80,
    "in_place": 80,
    "dash": 70,
    "slide": 55,
    "step": 40,
}

# 这些词更像攻击、跳跃或受击，不适合做基础闪避。
REJECT_KEYWORDS = (
    "attack",
    "slash",
    "hit",
    "impact",
    "death",
    "jump",
    "fall",
    "land",
)

# 当前闪避位移由 ARPGPlayerCharacter::LaunchCharacter 统一控制。
# 如果这里选择 RootMotion 动画，Mesh 可能先相对胶囊跑出去，动画结束再回到胶囊中心，看起来像镜头把角色强制拉回。
ROOT_MOTION_KEYWORDS = (
    "/rootmotion/",
    "/root_motion/",
    "_rootmotion",
    "_root_motion",
)


def log(message):
    unreal.log("[RPG Dash Animation] " + message)


def warn(message):
    unreal.log_warning("[RPG Dash Animation] " + message)


def load_asset(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if asset is None:
        raise RuntimeError(f"Missing asset: {path}")
    return asset


def score_animation(asset_data):
    package_name = str(asset_data.package_name).lower()
    asset_name = str(asset_data.asset_name).lower()
    text = package_name + "/" + asset_name

    # 攻击、跳跃、受击等排除词只看资产名，避免因为官方目录名如 Jump 误伤 MM_Dash。
    if any(keyword in asset_name for keyword in REJECT_KEYWORDS):
        return 0

    score = 0
    for keyword, value in PREFERRED_KEYWORDS.items():
        # 动作意图主要看资产名；目录名只用于来源和 RootMotion/InPlace 这类技术特征。
        if keyword in asset_name:
            score += value

    # 更偏向第三方动画包或导入目录，避免继续选到官方 MM_Dash。
    if "/characters/mannequins/anims/unarmed/jump/" in text:
        score -= 30

    # 当前免费包的闪避资源集中在 DashDodgeRollSet，路径本身比单个资产名更能说明用途。
    if "/dashdodgerollset/" in text:
        score += 60

    # 默认使用 LaunchCharacter 位移，所以自动选择时避开 RootMotion。
    # 如果要批量改成 RootMotion 闪避，可把 PREFER_ROOT_MOTION_DASH_ANIMATION 临时改为 True。
    if not PREFER_ROOT_MOTION_DASH_ANIMATION and any(keyword in text for keyword in ROOT_MOTION_KEYWORDS):
        score -= 300
    elif PREFER_ROOT_MOTION_DASH_ANIMATION and any(keyword in text for keyword in ROOT_MOTION_KEYWORDS):
        score += 120

    return max(score, 0)


def get_asset_object_path(asset_data):
    # UE 5.7 的 AssetData Python API 不再稳定提供 object_path 字段。
    # 用包名 + 资产名拼出 EditorAssetLibrary.load_asset 可识别的对象路径。
    return f"{asset_data.package_name}.{asset_data.asset_name}"


def find_best_dash_animation():
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    all_assets = asset_registry.get_assets_by_path("/Game", recursive=True)

    candidates = []
    for asset_data in all_assets:
        asset_class = str(asset_data.asset_class_path.asset_name)
        if asset_class not in ("AnimSequence", "AnimMontage"):
            continue

        score = score_animation(asset_data)
        if score <= 0:
            continue

        candidates.append((score, get_asset_object_path(asset_data), str(asset_data.asset_name)))

    candidates.sort(reverse=True)
    return candidates


def set_dash_animation(animation_path):
    blueprint = load_asset(PLAYER_BLUEPRINT_PATH)
    cdo = unreal.get_default_object(blueprint.generated_class())
    animation = load_asset(animation_path)

    cdo.set_editor_property("DashAnimation", animation)
    try:
        cdo.set_editor_property("bDashUseRootMotionAnimation", PREFER_ROOT_MOTION_DASH_ANIMATION)
    except Exception:
        warn("bDashUseRootMotionAnimation property not found. Recompile C++ and rerun this script if you want to set RootMotion mode from Python.")
    try:
        cdo.set_editor_property("DashAnimationSlotName", DASH_SLOT_NAME)
    except Exception:
        warn("DashAnimationSlotName property not found. Recompile C++ and rerun this script if you want to set the slot from Python.")
    try:
        fallback_animation = load_asset("/Game/Characters/Mannequins/Anims/Unarmed/Jump/MM_Dash.MM_Dash")
        cdo.set_editor_property("DashFallbackAnimation", fallback_animation)
    except Exception:
        warn("DashFallbackAnimation property not found. Recompile C++ and rerun this script if you want to set the fallback animation from Python.")

    unreal.EditorAssetLibrary.save_asset(PLAYER_BLUEPRINT_PATH)
    log(f"DashAnimation set to: {animation_path}")


def main():
    candidates = find_best_dash_animation()
    if not candidates:
        warn("No Dodge/Roll/Dash animation found under /Game. Import a free Fab animation pack first, then run this script again.")
        return

    log("Candidate animations:")
    for score, path, name in candidates[:10]:
        log(f"  score={score:03d} {name} -> {path}")

    _, best_path, _ = candidates[0]
    set_dash_animation(best_path)


if __name__ == "__main__":
    main()
