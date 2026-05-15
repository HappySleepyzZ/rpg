import unreal


MAP_PATH = "/Game/Maps/Prototype001"


def log(message):
    unreal.log("[RPG Inspect Prototype Map] " + message)


def main():
    unreal.EditorLevelLibrary.load_level(MAP_PATH)

    lines = []
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        label = actor.get_actor_label()
        actor_class = actor.get_class().get_name()
        location = actor.get_actor_location()
        lines.append(f"{label} | {actor_class} | ({location.x:.1f}, {location.y:.1f}, {location.z:.1f})")
        log(lines[-1])

    with open(r"G:\ue5_projects\rpg\rpg260514\Saved\prototype001_actors.txt", "w", encoding="utf-8") as handle:
        handle.write("\n".join(lines))


if __name__ == "__main__":
    main()
