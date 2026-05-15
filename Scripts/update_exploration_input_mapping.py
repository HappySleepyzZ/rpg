import unreal


def log(message):
    unreal.log("[RPG Input Mapping] " + message)


def load_asset(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if asset is None:
        raise RuntimeError(f"Missing asset: {path}")
    return asset


def key(name):
    result = unreal.Key()
    result.set_editor_property("key_name", name)
    return result


def make_modifier(modifier_class, outer):
    return unreal.new_object(modifier_class, outer=outer)


def make_negate(outer):
    return make_modifier(unreal.InputModifierNegate, outer)


def make_swizzle(outer):
    modifier = make_modifier(unreal.InputModifierSwizzleAxis, outer)
    modifier.set_editor_property("order", unreal.InputAxisSwizzle.YXZ)
    return modifier


def make_scalar(outer, scalar):
    modifier = make_modifier(unreal.InputModifierScalar, outer)
    modifier.set_editor_property("scalar", scalar)
    return modifier


def add_mapping(context, action, key_name, modifiers=None):
    mapping = context.map_key(action, key(key_name))
    if modifiers:
        mapping.set_editor_property("modifiers", modifiers)
    return mapping


def main():
    context = load_asset("/Game/Input/IMC_Exploration")
    move = load_asset("/Game/Input/Actions/IA_Move")
    look = load_asset("/Game/Input/Actions/IA_Look")
    jump = load_asset("/Game/Input/Actions/IA_Jump")
    interact = load_asset("/Game/Input/Actions/IA_Interact")
    primary = load_asset("/Game/Input/Actions/IA_PrimaryAction")
    # The asset is still named IA_Sprint, while the gameplay meaning is Dash.
    dash = load_asset("/Game/Input/Actions/IA_Sprint")
    zoom = load_asset("/Game/Input/Actions/IA_Zoom")

    # Clear old mappings before rebuilding the IMC so rerunning this script is deterministic.
    context.unmap_all_keys_from_action(move)
    context.unmap_all_keys_from_action(look)
    context.unmap_all_keys_from_action(jump)
    context.unmap_all_keys_from_action(interact)
    context.unmap_all_keys_from_action(primary)
    context.unmap_all_keys_from_action(dash)
    context.unmap_all_keys_from_action(zoom)

    # Keep this in sync with ARPGPlayerController::CreateRuntimeExplorationMappingContext
    # and ARPGPlayerController::HasRequiredExplorationMappings.
    add_mapping(context, move, "W", [make_swizzle(context)])
    add_mapping(context, move, "S", [make_negate(context), make_swizzle(context)])
    add_mapping(context, move, "A", [make_negate(context)])
    add_mapping(context, move, "D")
    add_mapping(context, move, "Gamepad_Left2D")

    add_mapping(context, look, "MouseX", [make_scalar(context, unreal.Vector(1.0, 0.0, 0.0))])
    add_mapping(context, look, "MouseY", [make_swizzle(context), make_scalar(context, unreal.Vector(0.0, -1.0, 0.0))])
    add_mapping(context, look, "Gamepad_Right2D")

    add_mapping(context, jump, "SpaceBar")
    add_mapping(context, jump, "Gamepad_FaceButton_Bottom")
    add_mapping(context, interact, "E")
    add_mapping(context, primary, "LeftMouseButton")
    add_mapping(context, dash, "LeftShift")
    add_mapping(context, zoom, "MouseWheelAxis")

    unreal.EditorAssetLibrary.save_asset("/Game/Input/IMC_Exploration")
    log("updated /Game/Input/IMC_Exploration")


if __name__ == "__main__":
    main()
