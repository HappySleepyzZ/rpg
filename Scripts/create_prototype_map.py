import unreal


MAP_PATH = "/Game/Maps/Prototype001"


def log(message):
    unreal.log("[RPG Prototype Map] " + message)


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def main():
    ensure_directory("/Game/Maps")

    if not unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
        unreal.EditorLevelLibrary.new_level(MAP_PATH)
        log(f"created {MAP_PATH}")
    else:
        unreal.EditorLevelLibrary.load_level(MAP_PATH)
        log(f"loaded {MAP_PATH}")

    # 命令行 commandlet 下 SpawnActor 在部分 UE 5.7 环境会触发 EditorScripting 崩溃。
    # 这里先只确保项目拥有稳定的默认地图资产；测试地面和关卡摆件后续在编辑器中迭代。
    unreal.EditorLevelLibrary.save_current_level()
    log(f"saved {MAP_PATH}")


if __name__ == "__main__":
    main()
