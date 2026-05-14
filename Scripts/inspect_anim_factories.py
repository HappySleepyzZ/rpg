import os
import unreal

output_path = os.path.join(unreal.Paths.project_saved_dir(), "AnimFactoryInspection.txt")

with open(output_path, "w", encoding="utf-8") as file:
    for name in dir(unreal):
        if "Anim" in name and "Factory" in name:
            file.write(name + "\n")

unreal.log("[RPG Anim Setup] wrote " + output_path)
