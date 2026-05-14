import os
import unreal

output_path = os.path.join(unreal.Paths.project_saved_dir(), "AnimBlueprintApiInspection.txt")

factory = unreal.AnimBlueprintFactory()
with open(output_path, "w", encoding="utf-8") as file:
    file.write("AnimBlueprintFactory properties/methods:\n")
    for name in dir(factory):
        if "class" in name.lower() or "skeleton" in name.lower() or "target" in name.lower() or "parent" in name.lower():
            file.write(name + "\n")

    file.write("\nAnimGraphLibrary names:\n")
    for name in dir(unreal):
        if "Anim" in name and ("Library" in name or "Graph" in name):
            file.write(name + "\n")

unreal.log("[RPG Anim Setup] wrote " + output_path)
