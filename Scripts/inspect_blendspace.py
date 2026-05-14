import os
import unreal

asset = unreal.EditorAssetLibrary.load_asset("/Game/Characters/Mannequins/Anims/Unarmed/BS_Idle_Walk_Run")
output_path = os.path.join(unreal.Paths.project_saved_dir(), "BlendSpaceInspection.txt")

with open(output_path, "w", encoding="utf-8") as file:
    file.write("asset: " + str(asset) + "\n")
    for index in range(3):
        try:
            param = asset.get_blend_parameter(index)
            file.write(f"param {index}: {param}\n")
        except Exception as exc:
            file.write(f"param {index} error: {exc}\n")
    for name in dir(asset):
        if "sample" in name.lower() or "blend" in name.lower() or "param" in name.lower():
            file.write("member: " + name + "\n")

unreal.log("[RPG BlendSpace Inspect] wrote " + output_path)
