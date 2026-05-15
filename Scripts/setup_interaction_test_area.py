import unreal


MAP_PATH = "/Game/Maps/Prototype001"
BLUEPRINT_PATH = "/Game/Interaction/BP_TestOneShotInteractable"
ACTOR_LABEL = "Test One Shot Interactable"
PLAYER_START_LABEL = "Prototype PlayerStart"
GROUND_LABEL = "Prototype Ground"
LIGHT_LABEL = "Prototype Directional Light"
SKY_LIGHT_LABEL = "Prototype Sky Light"
FOG_LABEL = "Prototype Exponential Height Fog"
ATMOSPHERE_LABEL = "Prototype Sky Atmosphere"
SKY_SPHERE_LABEL = "Prototype Sky Sphere"
EMISSIVE_MATERIAL_PATH = "/Game/Interaction/M_TestInteractable_Visible"


def log(message):
    unreal.log("[RPG Interaction Test Area] " + message)


def load_asset(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if asset is None:
        raise RuntimeError(f"Missing asset: {path}")
    return asset


def load_first_existing_asset(paths):
    for path in paths:
        if unreal.EditorAssetLibrary.does_asset_exist(path):
            asset = unreal.EditorAssetLibrary.load_asset(path)
            if asset is not None:
                return asset
    return None


def ensure_emissive_material():
    if unreal.EditorAssetLibrary.does_asset_exist(EMISSIVE_MATERIAL_PATH):
        return load_asset(EMISSIVE_MATERIAL_PATH)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.MaterialFactoryNew()
    material = asset_tools.create_asset("M_TestInteractable_Visible", "/Game/Interaction", unreal.Material, factory)
    if material is None:
        raise RuntimeError("Failed to create visible test material")

    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_OPAQUE)
    material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)

    color_expression = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionConstant4Vector, -400, 0)
    color_expression.set_editor_property("constant", unreal.LinearColor(0.0, 1.0, 0.1, 1.0))
    unreal.MaterialEditingLibrary.connect_material_property(color_expression, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material


def find_actor(label):
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        if actor.get_actor_label() == label:
            return actor
    return None


def set_transform(actor, location, rotation=None, scale=None):
    actor.set_actor_location(location, False, False)
    if rotation is not None:
        actor.set_actor_rotation(rotation, False)
    if scale is not None:
        actor.set_actor_scale3d(scale)


def count_existing_floor_like_actors():
    floor_keywords = ("floor", "ground", "plane", "grid")
    count = 0
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        label = actor.get_actor_label().lower()
        actor_class = actor.get_class().get_name().lower()
        if any(keyword in label or keyword in actor_class for keyword in floor_keywords):
            count += 1
    return count


def ensure_ground():
    if count_existing_floor_like_actors() > 0:
        log("existing floor-like actor found; prototype ground not added")
        return

    actor = find_actor(GROUND_LABEL)
    if actor is None:
        actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(0.0, 0.0, -5.0),
            unreal.Rotator(0.0, 0.0, 0.0),
        )
        actor.set_actor_label(GROUND_LABEL)

    static_mesh_component = actor.get_editor_property("static_mesh_component")
    static_mesh = load_first_existing_asset([
        "/Game/FreeSampleAnimationSet/Demo/LevelPrototyping/Meshes/SM_Cube.SM_Cube",
        "/Engine/BasicShapes/Cube.Cube",
    ])
    if static_mesh is not None:
        static_mesh_component.set_editor_property("static_mesh", static_mesh)

    material = load_first_existing_asset([
        "/Game/FreeSampleAnimationSet/Demo/LevelPrototyping/Materials/MI_PrototypeGrid_Gray.MI_PrototypeGrid_Gray",
        "/Engine/EngineMaterials/WorldGridMaterial.WorldGridMaterial",
    ])
    if material is not None:
        static_mesh_component.set_material(0, material)

    static_mesh_component.set_editor_property("mobility", unreal.ComponentMobility.STATIC)
    set_transform(actor, unreal.Vector(0.0, 0.0, -10.0), unreal.Rotator(0.0, 0.0, 0.0), unreal.Vector(24.0, 24.0, 0.2))
    log("ground at (0, 0, -10), scale 24 x 24")


def ensure_lighting():
    light = find_actor(LIGHT_LABEL)
    directional_light_class = unreal.load_class(None, "/Script/Engine.DirectionalLight")
    if light is None:
        light = unreal.EditorLevelLibrary.spawn_actor_from_class(
            directional_light_class,
            unreal.Vector(-250.0, -250.0, 450.0),
            unreal.Rotator(-45.0, -35.0, 0.0),
        )
        light.set_actor_label(LIGHT_LABEL)
    set_transform(light, unreal.Vector(-250.0, -250.0, 450.0), unreal.Rotator(-45.0, -35.0, 0.0))
    light_component = light.get_editor_property("light_component")
    light_component.set_editor_property("intensity", 10.0)

    sky_light = find_actor(SKY_LIGHT_LABEL)
    sky_light_class = unreal.load_class(None, "/Script/Engine.SkyLight")
    if sky_light is None:
        sky_light = unreal.EditorLevelLibrary.spawn_actor_from_class(
            sky_light_class,
            unreal.Vector(0.0, 0.0, 250.0),
            unreal.Rotator(0.0, 0.0, 0.0),
        )
        sky_light.set_actor_label(SKY_LIGHT_LABEL)
    sky_component = sky_light.get_editor_property("light_component")
    sky_component.set_editor_property("intensity", 1.5)
    sky_component.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)

    atmosphere = find_actor(ATMOSPHERE_LABEL)
    atmosphere_class = unreal.load_class(None, "/Script/Engine.SkyAtmosphere")
    if atmosphere is None:
        atmosphere = unreal.EditorLevelLibrary.spawn_actor_from_class(
            atmosphere_class,
            unreal.Vector(0.0, 0.0, 0.0),
            unreal.Rotator(0.0, 0.0, 0.0),
        )
        atmosphere.set_actor_label(ATMOSPHERE_LABEL)

    fog = find_actor(FOG_LABEL)
    fog_class = unreal.load_class(None, "/Script/Engine.ExponentialHeightFog")
    if fog is None:
        fog = unreal.EditorLevelLibrary.spawn_actor_from_class(
            fog_class,
            unreal.Vector(0.0, 0.0, 0.0),
            unreal.Rotator(0.0, 0.0, 0.0),
        )
        fog.set_actor_label(FOG_LABEL)

    sky_sphere = find_actor(SKY_SPHERE_LABEL)
    sky_mesh = load_first_existing_asset([
        "/Engine/EngineSky/SM_SkySphere.SM_SkySphere",
        "/Engine/EngineSky/SM_SkyDome.SM_SkyDome",
    ])
    if sky_mesh is not None:
        if sky_sphere is None:
            sky_sphere = unreal.EditorLevelLibrary.spawn_actor_from_class(
                unreal.StaticMeshActor,
                unreal.Vector(0.0, 0.0, 0.0),
                unreal.Rotator(0.0, 0.0, 0.0),
            )
            sky_sphere.set_actor_label(SKY_SPHERE_LABEL)

        sky_mesh_component = sky_sphere.get_editor_property("static_mesh_component")
        sky_mesh_component.set_editor_property("static_mesh", sky_mesh)
        sky_mesh_component.set_editor_property("mobility", unreal.ComponentMobility.STATIC)
        set_transform(sky_sphere, unreal.Vector(0.0, 0.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0), unreal.Vector(400.0, 400.0, 400.0))

    log("lighting added")


def ensure_player_start():
    actor = find_actor(PLAYER_START_LABEL)
    if actor is None:
        player_start_class = unreal.load_class(None, "/Script/Engine.PlayerStart")
        actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
            player_start_class,
            unreal.Vector(0.0, 0.0, 110.0),
            unreal.Rotator(0.0, 0.0, 0.0),
        )
        actor.set_actor_label(PLAYER_START_LABEL)
    else:
        set_transform(actor, unreal.Vector(0.0, 0.0, 110.0), unreal.Rotator(0.0, 0.0, 0.0))

    log("player start at (0, 0, 110)")


def ensure_interactable():
    blueprint = load_asset(BLUEPRINT_PATH)
    actor_class = blueprint.generated_class()
    visible_material = ensure_emissive_material()

    actor = find_actor(ACTOR_LABEL)
    if actor is None:
        actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
            actor_class,
            unreal.Vector(180.0, 0.0, 0.0),
            unreal.Rotator(0.0, 0.0, 0.0),
        )
        actor.set_actor_label(ACTOR_LABEL)
    else:
        set_transform(actor, unreal.Vector(180.0, 0.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0), unreal.Vector(1.0, 1.0, 1.0))

    try:
        actor.set_editor_property("can_interact", True)
        actor.set_editor_property("disable_after_interaction", True)
        visual_mesh = actor.get_editor_property("visual_mesh")
        visual_mesh.set_editor_property("static_mesh", load_asset("/Engine/BasicShapes/Cube.Cube"))
        visual_mesh.set_editor_property("relative_location", unreal.Vector(0.0, 0.0, 120.0))
        visual_mesh.set_editor_property("relative_scale3d", unreal.Vector(2.4, 2.4, 2.4))
        visual_mesh.set_material(0, visible_material)

        interaction_bounds = actor.get_editor_property("interaction_bounds")
        interaction_bounds.set_editor_property("sphere_radius", 360.0)
    except Exception as exc:
        log(f"runtime property reset skipped: {exc}")

    log("interactable at (180, 0, 0), visual cube is 240 x 240 x 240")


def remove_template_duplicates():
    # Keep Prototype001 as the only canonical test map, but avoid duplicate default-lighting actors
    # created by previous attempts or copied template content.
    managed_labels = {
        GROUND_LABEL,
        LIGHT_LABEL,
        SKY_LIGHT_LABEL,
        FOG_LABEL,
        ATMOSPHERE_LABEL,
        SKY_SPHERE_LABEL,
        PLAYER_START_LABEL,
        ACTOR_LABEL,
    }

    seen_labels = set()
    for actor in list(unreal.EditorLevelLibrary.get_all_level_actors()):
        label = actor.get_actor_label()
        if label in managed_labels:
            if label in seen_labels:
                unreal.EditorLevelLibrary.destroy_actor(actor)
            seen_labels.add(label)


def main():
    unreal.EditorLevelLibrary.load_level(MAP_PATH)
    remove_template_duplicates()
    ensure_ground()
    ensure_lighting()
    ensure_player_start()
    ensure_interactable()
    unreal.EditorLevelLibrary.save_current_level()
    log(f"saved {MAP_PATH}")


if __name__ == "__main__":
    main()
