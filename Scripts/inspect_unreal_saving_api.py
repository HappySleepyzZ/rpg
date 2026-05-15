import unreal
import inspect

lines = []

for obj in (unreal.EditorLoadingAndSavingUtils, unreal.LevelEditorSubsystem):
    lines.append(f"[API Inspect] {obj}")
    names = [name for name in dir(obj) if "map" in name.lower() or "level" in name.lower() or "save" in name.lower() or "load" in name.lower()]
    lines.append("[API Inspect] " + ", ".join(sorted(names)))

for func in (
    unreal.EditorLoadingAndSavingUtils.new_map_from_template,
    unreal.EditorLoadingAndSavingUtils.save_map,
    unreal.EditorLoadingAndSavingUtils.load_map,
    unreal.LevelEditorSubsystem.new_level_from_template,
):
    try:
        lines.append(f"[API Inspect] signature {func}: {inspect.signature(func)}")
    except Exception as exc:
        lines.append(f"[API Inspect] signature failed {func}: {exc}")
    lines.append(f"[API Inspect] doc {func}: {getattr(func, '__doc__', '')}")

with open(r"G:\ue5_projects\rpg\rpg260514\Saved\api_inspect.txt", "w", encoding="utf-8") as handle:
    handle.write("\n".join(lines))
