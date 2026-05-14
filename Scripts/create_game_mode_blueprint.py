import unreal


def log(message):
    unreal.log("[RPG GameMode Blueprint] " + message)


def load_asset(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if asset is None:
        raise RuntimeError(f"Missing asset: {path}")
    return asset


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def main():
    ensure_directory("/Game/Core")

    bp_path = "/Game/Core/BP_RPGGameMode"
    if unreal.EditorAssetLibrary.does_asset_exist(bp_path):
        blueprint = load_asset(bp_path)
        log(f"{bp_path} already exists")
    else:
        parent_class = unreal.load_class(None, "/Script/rpg260514.RPGGameMode")
        factory = unreal.BlueprintFactory()
        factory.set_editor_property("parent_class", parent_class)
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        blueprint = asset_tools.create_asset("BP_RPGGameMode", "/Game/Core", None, factory)
        if blueprint is None:
            raise RuntimeError("Failed to create BP_RPGGameMode")
        log("created BP_RPGGameMode")

    player_bp = load_asset("/Game/Characters/Player/BP_PlayerCharacter")
    game_mode_cdo = unreal.get_default_object(blueprint.generated_class())
    game_mode_cdo.set_editor_property("default_pawn_class", player_bp.generated_class())

    unreal.EditorAssetLibrary.save_asset(bp_path)
    log("configured BP_RPGGameMode default pawn")


if __name__ == "__main__":
    main()
