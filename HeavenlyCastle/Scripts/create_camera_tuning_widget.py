# -*- coding: ascii -*-
import unreal

ASSET_PATH = "/Game/Editor/Utility"
ASSET_NAME = "EUW_PlayerCameraTuning"
FULL_PATH = f"{ASSET_PATH}/{ASSET_NAME}"


def create_widget():
    if unreal.EditorAssetLibrary.does_asset_exist(FULL_PATH):
        unreal.log(f"[CameraTuningWidget] Asset already exists at {FULL_PATH}")
        return unreal.EditorAssetLibrary.load_asset(FULL_PATH)

    if not unreal.EditorAssetLibrary.does_directory_exist(ASSET_PATH):
        unreal.EditorAssetLibrary.make_directory(ASSET_PATH)

    factory = unreal.EditorUtilityWidgetBlueprintFactory()
    factory.set_editor_property("ParentClass", unreal.EditorPlayerCameraTuningWidget)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    widget_bp = asset_tools.create_asset(
        ASSET_NAME,
        ASSET_PATH,
        unreal.EditorUtilityWidgetBlueprint,
        factory
    )

    if widget_bp:
        widget_bp.mark_package_dirty()
        unreal.log(f"[CameraTuningWidget] Created asset: {FULL_PATH}")
    else:
        unreal.log_error("[CameraTuningWidget] Failed to create editor utility widget blueprint.")

    return widget_bp


def spawn_widget(widget_asset):
    subsystem = unreal.get_editor_subsystem(unreal.EditorUtilitySubsystem)
    if not subsystem:
        unreal.log_error("[CameraTuningWidget] EditorUtilitySubsystem not available")
        return

    subsystem.spawn_and_register_tab(widget_asset)
    unreal.log("[CameraTuningWidget] Widget tab spawned")


def main():
    widget_bp = create_widget()
    if widget_bp:
        spawn_widget(widget_bp)


if __name__ == "__main__":
    main()

