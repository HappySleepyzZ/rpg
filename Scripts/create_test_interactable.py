import unreal


MAP_PATH = "/Game/Maps/Prototype001"
BLUEPRINT_DIR = "/Game/Interaction"
BLUEPRINT_NAME = "BP_TestOneShotInteractable"
BLUEPRINT_PATH = f"{BLUEPRINT_DIR}/{BLUEPRINT_NAME}"
ACTOR_LABEL = "Test One Shot Interactable"


def log(message):
    unreal.log("[RPG Test Interactable] " + message)


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def load_asset(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if asset is None:
        raise RuntimeError(f"Missing asset: {path}")
    return asset


def get_generated_class(blueprint):
    return blueprint.generated_class()


def create_or_update_blueprint():
    ensure_directory(BLUEPRINT_DIR)

    if unreal.EditorAssetLibrary.does_asset_exist(BLUEPRINT_PATH):
        blueprint = load_asset(BLUEPRINT_PATH)
        log(f"{BLUEPRINT_PATH} already exists")
    else:
        parent_class = unreal.load_class(None, "/Script/rpg260514.RPGInteractableActor")
        if parent_class is None:
            raise RuntimeError("Missing C++ class /Script/rpg260514.RPGInteractableActor. Compile the project before running this script.")

        factory = unreal.BlueprintFactory()
        factory.set_editor_property("parent_class", parent_class)
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        blueprint = asset_tools.create_asset(BLUEPRINT_NAME, BLUEPRINT_DIR, None, factory)
        if blueprint is None:
            raise RuntimeError(f"Failed to create {BLUEPRINT_PATH}")
        log(f"created {BLUEPRINT_PATH}")

    cdo = unreal.get_default_object(get_generated_class(blueprint))
    cdo.set_editor_property("interaction_prompt", unreal.Text("Press E to test"))
    cdo.set_editor_property("used_interaction_prompt", unreal.Text("Already used"))
    cdo.set_editor_property("disable_after_interaction", True)
    cdo.set_editor_property("log_debug_interaction", True)

    try:
        mesh_component = cdo.get_editor_property("visual_mesh")
        mesh_component.set_editor_property("static_mesh", load_asset("/Engine/BasicShapes/Cube.Cube"))
        mesh_component.set_editor_property("relative_location", unreal.Vector(0.0, 0.0, 100.0))
        mesh_component.set_editor_property("relative_scale3d", unreal.Vector(1.6, 1.6, 2.0))

        interaction_bounds = cdo.get_editor_property("interaction_bounds")
        interaction_bounds.set_editor_property("sphere_radius", 260.0)
    except Exception as exc:
        log(f"visual mesh setup skipped: {exc}")

    unreal.EditorAssetLibrary.save_loaded_asset(blueprint)
    return blueprint


def find_existing_actor():
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        if actor.get_actor_label() == ACTOR_LABEL:
            return actor
    return None


def place_actor(blueprint):
    unreal.EditorLevelLibrary.load_level(MAP_PATH)

    actor_class = get_generated_class(blueprint)
    actor = find_existing_actor()
    if actor is None:
        actor = unreal.EditorLevelLibrary.spawn_actor_from_class(actor_class, unreal.Vector(180.0, 0.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0))
        actor.set_actor_label(ACTOR_LABEL)
        log(f"spawned {ACTOR_LABEL}")
    else:
        actor.set_actor_location(unreal.Vector(180.0, 0.0, 0.0), False, False)
        actor.set_actor_rotation(unreal.Rotator(0.0, 0.0, 0.0), False)
        log(f"updated {ACTOR_LABEL}")

    unreal.EditorLevelLibrary.save_current_level()
    log(f"saved {MAP_PATH}")


def main():
    blueprint = create_or_update_blueprint()
    place_actor(blueprint)


if __name__ == "__main__":
    main()
