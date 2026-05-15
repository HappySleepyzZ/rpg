import unreal


OPENWORLD_TEMPLATE_FILE = r"g:\Program Files\Epic Games\UE_5.7\Engine\Content\Maps\Templates\OpenWorld.umap"
MAP_PATH = "/Game/Maps/Prototype001"
BLUEPRINT_PATH = "/Game/Interaction/BP_TestOneShotInteractable"
VISIBLE_MATERIAL_PATH = "/Game/Interaction/M_TestInteractable_Visible"
ACTOR_LABEL = "Test One Shot Interactable"


def log(message):
    unreal.log("[RPG Rebuild Prototype001] " + message)


def load_asset(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if asset is None:
        raise RuntimeError(f"Missing asset: {path}")
    return asset


def load_optional_asset(path):
    return unreal.EditorAssetLibrary.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else None


def create_map_from_template():
    # This is the same source UE uses when the editor shows a fresh "Untitled" Open World map.
    # The autosaved Untitled shell does not reliably carry its external actors into /Game.
    world = unreal.EditorLoadingAndSavingUtils.new_map_from_template(OPENWORLD_TEMPLATE_FILE, False)
    if world is None:
        raise RuntimeError(f"Failed to create map from template file: {OPENWORLD_TEMPLATE_FILE}")

    log(f"created transient map from {OPENWORLD_TEMPLATE_FILE}")
    return world


def get_all_actors():
    return list(unreal.EditorLevelLibrary.get_all_level_actors())


def find_actor_by_label(label):
    for actor in get_all_actors():
        if actor.get_actor_label() == label:
            return actor
    return None


def find_player_start():
    for actor in get_all_actors():
        if actor.get_class().get_name() == "PlayerStart":
            return actor
    return None


def ensure_player_start():
    actor = find_player_start()
    if actor is not None:
        return actor

    player_start_class = unreal.load_class(None, "/Script/Engine.PlayerStart")
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        player_start_class,
        unreal.Vector(0.0, 0.0, 120.0),
        unreal.Rotator(0.0, 0.0, 0.0),
    )
    actor.set_actor_label("Prototype PlayerStart")
    log("added PlayerStart at (0, 0, 120)")
    return actor


def add_interactable():
    blueprint = load_asset(BLUEPRINT_PATH)
    actor_class = blueprint.generated_class()
    player_start = ensure_player_start()
    start_location = player_start.get_actor_location()
    start_rotation = player_start.get_actor_rotation()

    yaw = start_rotation.yaw
    forward = unreal.Rotator(0.0, yaw, 0.0).get_forward_vector()
    location = start_location + (forward * 260.0)
    location.z = 0.0

    existing = find_actor_by_label(ACTOR_LABEL)
    if existing is not None:
        unreal.EditorLevelLibrary.destroy_actor(existing)

    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        actor_class,
        location,
        unreal.Rotator(0.0, 0.0, 0.0),
    )
    actor.set_actor_label(ACTOR_LABEL)
    try:
        actor.set_editor_property("is_spatially_loaded", False)
    except Exception as exc:
        log(f"non-spatial flag skipped: {exc}")

    try:
        actor.set_editor_property("can_interact", True)
        actor.set_editor_property("disable_after_interaction", True)
        visual_mesh = actor.get_editor_property("visual_mesh")
        visual_mesh.set_editor_property("relative_location", unreal.Vector(0.0, 0.0, 150.0))
        visual_mesh.set_editor_property("relative_scale3d", unreal.Vector(3.0, 3.0, 3.0))
        visible_material = load_optional_asset(VISIBLE_MATERIAL_PATH)
        if visible_material is not None:
            visual_mesh.set_material(0, visible_material)
        interaction_bounds = actor.get_editor_property("interaction_bounds")
        interaction_bounds.set_editor_property("sphere_radius", 420.0)
    except Exception as exc:
        log(f"interaction property reset skipped: {exc}")

    log(
        "spawned one-shot interactable at "
        f"({location.x:.1f}, {location.y:.1f}, {location.z:.1f})"
    )


def main():
    world = create_map_from_template()
    add_interactable()
    if not unreal.EditorLoadingAndSavingUtils.save_map(world, MAP_PATH):
        raise RuntimeError(f"Failed to save map to {MAP_PATH}")
    log(f"saved {MAP_PATH}")


if __name__ == "__main__":
    main()
