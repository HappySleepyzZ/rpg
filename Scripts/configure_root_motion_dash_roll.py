import unreal


PLAYER_BLUEPRINT_PATH = "/Game/Characters/Player/BP_PlayerCharacter"
ROLL_ANIMATION_PATH = "/Game/FreeSampleAnimationSet/Animations/DashDodgeRollSet/Mannequin/RootMotion/Roll/A_Roll_IdleFwd.A_Roll_IdleFwd"
FALLBACK_ANIMATION_PATH = "/Game/Characters/Mannequins/Anims/Unarmed/Jump/MM_Dash.MM_Dash"


def log(message):
    unreal.log("[RPG RootMotion Dash] " + message)


def warn(message):
    unreal.log_warning("[RPG RootMotion Dash] " + message)


def load_asset(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if asset is None:
        raise RuntimeError(f"Missing asset: {path}")
    return asset


def set_property_if_available(asset, property_name, value):
    try:
        old_value = asset.get_editor_property(property_name)
        asset.set_editor_property(property_name, value)
        log(f"{asset.get_name()}.{property_name}: {old_value} -> {value}")
        return True
    except Exception as error:
        warn(f"Skip {asset.get_name()}.{property_name}: {error}")
        return False


def configure_roll_animation():
    roll_animation = load_asset(ROLL_ANIMATION_PATH)

    # 路径里带 RootMotion 不代表资产已经启用 Root Motion 提取。
    # 如果这里没开，Mesh 会自己滚出去，胶囊和相机会留在原地，动画结束后就会视觉回弹。
    set_property_if_available(roll_animation, "enable_root_motion", True)
    set_property_if_available(roll_animation, "force_root_lock", False)

    unreal.EditorAssetLibrary.save_loaded_asset(roll_animation)
    return roll_animation


def configure_player_blueprint(roll_animation):
    player_blueprint = load_asset(PLAYER_BLUEPRINT_PATH)
    fallback_animation = load_asset(FALLBACK_ANIMATION_PATH)
    cdo = unreal.get_default_object(player_blueprint.generated_class())

    cdo.set_editor_property("DashAnimation", roll_animation)
    cdo.set_editor_property("DashFallbackAnimation", fallback_animation)
    cdo.set_editor_property("bDashUseRootMotionAnimation", True)
    cdo.set_editor_property("DashAnimationSlotName", "DefaultSlot")

    unreal.EditorAssetLibrary.save_asset(PLAYER_BLUEPRINT_PATH)
    log("BP_PlayerCharacter configured for RootMotion Roll dash.")


def main():
    roll_animation = configure_roll_animation()
    configure_player_blueprint(roll_animation)


if __name__ == "__main__":
    main()
