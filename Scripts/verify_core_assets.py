import unreal


REQUIRED_ASSETS = [
    "/Game/Characters/Player/BP_PlayerCharacter.BP_PlayerCharacter",
    "/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple",
    "/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed",
    "/Game/Characters/Mannequins/Anims/Unarmed/Jump/MM_Dash.MM_Dash",
    "/Game/FreeSampleAnimationSet/Animations/DashDodgeRollSet/Mannequin/RootMotion/Roll/A_Roll_IdleFwd.A_Roll_IdleFwd",
]


def log(message):
    unreal.log("[RPG Verify Core Assets] " + message)


def main():
    missing_assets = []
    for asset_path in REQUIRED_ASSETS:
        if unreal.EditorAssetLibrary.load_asset(asset_path) is None:
            missing_assets.append(asset_path)

    legacy_assets = unreal.EditorAssetLibrary.list_assets("/Game/Mannequins", recursive=True, include_folder=True)

    if missing_assets:
        raise RuntimeError("Missing required assets:\n" + "\n".join(missing_assets))

    if legacy_assets:
        raise RuntimeError("Legacy /Game/Mannequins assets still exist:\n" + "\n".join(legacy_assets))

    log("core assets loaded successfully; legacy /Game/Mannequins directory is gone")


if __name__ == "__main__":
    main()
