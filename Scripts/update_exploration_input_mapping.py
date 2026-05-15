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
    move = load_asset("/Game/Input/Actions/IA_Move")
    look = load_asset("/Game/Input/Actions/IA_Look")
    jump = load_asset("/Game/Input/Actions/IA_Jump")
    interact = load_asset("/Game/Input/Actions/IA_Interact")
    primary = load_asset("/Game/Input/Actions/IA_PrimaryAction")
    sprint = load_asset("/Game/Input/Actions/IA_Sprint")

    # 重复运行脚本时先清掉旧映射，避免 IMC 里出现重复按键。
    context.unmap_all_keys_from_action(move)
    context.unmap_all_keys_from_action(look)
    context.unmap_all_keys_from_action(jump)
    context.unmap_all_keys_from_action(interact)
    context.unmap_all_keys_from_action(primary)
    context.unmap_all_keys_from_action(sprint)

    context.map_key(move, key("W"))
    context.map_key(move, key("S"))
    context.map_key(move, key("A"))
    context.map_key(move, key("D"))
    context.map_key(move, key("Gamepad_Left2D"))
    context.map_key(look, key("Mouse2D"))
    context.map_key(look, key("Gamepad_Right2D"))
    context.map_key(jump, key("SpaceBar"))
    context.map_key(jump, key("Gamepad_FaceButton_Bottom"))
    context.map_key(interact, key("E"))
    context.map_key(primary, key("LeftMouseButton"))
    context.map_key(sprint, key("LeftShift"))

    unreal.EditorAssetLibrary.save_asset("/Game/Input/IMC_Exploration")
    log("updated /Game/Input/IMC_Exploration")


if __name__ == "__main__":
    main()
