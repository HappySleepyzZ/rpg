import unreal


def log(message):
    unreal.log("[RPG Input Mapping] " + message)


def load_asset(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if asset is None:
        raise RuntimeError(f"Missing asset: {path}")
    return asset


def key(name):
    return unreal.Key(name)


def main():
    context = load_asset("/Game/Input/IMC_Exploration")
    interact = load_asset("/Game/Input/Actions/IA_Interact")
    primary = load_asset("/Game/Input/Actions/IA_PrimaryAction")

    # 重复运行脚本时先清掉这两个动作的旧映射，避免 IMC 里出现重复按键。
    context.unmap_all_keys_from_action(interact)
    context.unmap_all_keys_from_action(primary)

    context.map_key(interact, key("E"))
    context.map_key(primary, key("LeftMouseButton"))

    unreal.EditorAssetLibrary.save_asset("/Game/Input/IMC_Exploration")
    log("updated /Game/Input/IMC_Exploration")


if __name__ == "__main__":
    main()
