import unreal


def log(message):
    unreal.log("[RPG Player Blueprint] " + message)


def load_asset(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if asset is None:
        raise RuntimeError(f"Missing asset: {path}")
    return asset


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def get_generated_class(blueprint):
    return blueprint.generated_class()


def main():
    ensure_directory("/Game/Characters/Player")

    bp_path = "/Game/Characters/Player/BP_PlayerCharacter"
    if unreal.EditorAssetLibrary.does_asset_exist(bp_path):
        blueprint = load_asset(bp_path)
        log(f"{bp_path} already exists")
    else:
        parent_class = unreal.load_class(None, "/Script/rpg260514.RPGPlayerCharacter")
        factory = unreal.BlueprintFactory()
        factory.set_editor_property("parent_class", parent_class)
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        blueprint = asset_tools.create_asset("BP_PlayerCharacter", "/Game/Characters/Player", None, factory)
        if blueprint is None:
            raise RuntimeError("Failed to create BP_PlayerCharacter")
        log("created BP_PlayerCharacter")

    # 修改蓝图 CDO，避免依赖打开蓝图编辑器手工设置默认值。
    generated_class = get_generated_class(blueprint)
    cdo = unreal.get_default_object(generated_class)

    mesh_component = cdo.get_editor_property("mesh")
    mesh_component.set_editor_property("skeletal_mesh", load_asset("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"))
    mesh_component.set_editor_property("relative_location", unreal.Vector(0.0, 0.0, -95.0))
    mesh_component.set_editor_property("relative_rotation", unreal.Rotator(0.0, 0.0, -90.0))

    anim_blueprint = load_asset("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed")
    mesh_component.set_editor_property("animation_mode", unreal.AnimationMode.ANIMATION_BLUEPRINT)
    mesh_component.set_editor_property("anim_class", anim_blueprint.generated_class())

    unreal.EditorAssetLibrary.save_asset(bp_path)

    # 让项目默认直接使用带 Mesh/动画的玩家蓝图。
    game_mode_class = unreal.load_class(None, "/Script/rpg260514.RPGGameMode")
    game_mode_cdo = unreal.get_default_object(game_mode_class)
    game_mode_cdo.set_editor_property("default_pawn_class", generated_class)

    unreal.EditorAssetLibrary.save_loaded_asset(blueprint)
    log("configured BP_PlayerCharacter and RPGGameMode default pawn")


if __name__ == "__main__":
    main()
