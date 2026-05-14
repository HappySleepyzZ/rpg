import unreal


def log(message):
    unreal.log("[RPG Input Setup] " + message)


def ensure_package_path(path):
    editor_asset_library = unreal.EditorAssetLibrary
    if not editor_asset_library.does_directory_exist(path):
        editor_asset_library.make_directory(path)


def create_input_action(asset_name, value_type=None):
    asset_path = f"/Game/Input/Actions/{asset_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        log(f"{asset_path} already exists")
        return unreal.EditorAssetLibrary.load_asset(asset_path)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    asset = asset_tools.create_asset(asset_name, "/Game/Input/Actions", unreal.InputAction, None)
    if asset is None:
        raise RuntimeError(f"Failed to create {asset_path}")

    if value_type is not None:
        # UE 5.x exposes value_type as an enum property on UInputAction.
        asset.set_editor_property("value_type", value_type)

    unreal.EditorAssetLibrary.save_asset(asset_path)
    log(f"created {asset_path}")
    return asset


def main():
    ensure_package_path("/Game/Input")
    ensure_package_path("/Game/Input/Actions")

    bool_type = None
    if hasattr(unreal, "InputActionValueType"):
        # Boolean actions are used for one-shot buttons such as interact and primary action.
        bool_type = unreal.InputActionValueType.BOOLEAN

    create_input_action("IA_Interact", bool_type)
    create_input_action("IA_PrimaryAction", bool_type)

    # Rename the official default context to our project naming if possible.
    # This keeps a stable RPG-facing name while preserving UE-created action assets.
    if unreal.EditorAssetLibrary.does_asset_exist("/Game/Input/IMC_Default") and not unreal.EditorAssetLibrary.does_asset_exist("/Game/Input/IMC_Exploration"):
        unreal.EditorAssetLibrary.rename_asset("/Game/Input/IMC_Default", "/Game/Input/IMC_Exploration")
        log("renamed /Game/Input/IMC_Default to /Game/Input/IMC_Exploration")

    unreal.EditorAssetLibrary.save_directory("/Game/Input")


if __name__ == "__main__":
    main()
