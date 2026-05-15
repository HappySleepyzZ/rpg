import unreal


MAP_PATH = "/Game/Maps/Prototype001"
BLUEPRINT_PATH = "/Game/Interaction/BP_TestOneShotInteractable"
ACTOR_LABEL = "Test One Shot Interactable"
PLAYER_START_LABEL = "Prototype PlayerStart"


def log(message):
    unreal.log("[RPG Add Interactable Only] " + message)


def load_asset(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if asset is None:
        raise RuntimeError(f"Missing asset: {path}")
    return asset


def find_actor(label):
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        if actor.get_actor_label() == label:
            return actor
    return None


def find_any_player_start():
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        if actor.get_class().get_name() == "PlayerStart" or "PlayerStart" in actor.get_actor_label():
            return actor
    return None


def ensure_player_start():
    actor = find_any_player_start()
    if actor is None:
        player_start_class = unreal.load_class(None, "/Script/Engine.PlayerStart")
        actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
            player_start_class,
            unreal.Vector(0.0, 0.0, 110.0),
            unreal.Rotator(0.0, 0.0, 0.0),
        )
        actor.set_actor_label(PLAYER_START_LABEL)
        log("added PlayerStart at (0, 0, 110)")
    else:
        location = actor.get_actor_location()
        log(f"kept existing PlayerStart at ({location.x:.1f}, {location.y:.1f}, {location.z:.1f})")


def add_or_update_interactable():
    blueprint = load_asset(BLUEPRINT_PATH)
    actor_class = blueprint.generated_class()

    actor = find_actor(ACTOR_LABEL)
    if actor is None:
        actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
            actor_class,
            unreal.Vector(220.0, 0.0, 0.0),
            unreal.Rotator(0.0, 0.0, 0.0),
        )
        actor.set_actor_label(ACTOR_LABEL)
        log("spawned interactable at (220, 0, 0)")
    else:
        actor.set_actor_location(unreal.Vector(220.0, 0.0, 0.0), False, False)
        actor.set_actor_rotation(unreal.Rotator(0.0, 0.0, 0.0), False)
        log("moved interactable to (220, 0, 0)")

    try:
        actor.set_editor_property("can_interact", True)
        actor.set_editor_property("disable_after_interaction", True)
    except Exception as exc:
        log(f"interaction property reset skipped: {exc}")


def main():
    unreal.EditorLevelLibrary.load_level(MAP_PATH)
    ensure_player_start()
    add_or_update_interactable()
    unreal.EditorLevelLibrary.save_current_level()
    log(f"saved {MAP_PATH}")


if __name__ == "__main__":
    main()
