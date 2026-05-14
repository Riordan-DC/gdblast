#include "register_types.h"

#include "gdblast/GodotBlast.h"
#include "gdblast/GDBlastSingleton.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/engine.hpp>

using namespace godot;

static NvBlastFramework* framework;

void initialize_example_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

    // Register Godot C++ classes here
    GDREGISTER_CLASS(BlastActor);
	GDREGISTER_CLASS(BlastGroup);
	GDREGISTER_CLASS(BlastChunk);
	GDREGISTER_CLASS(BlastFamily);
	GDREGISTER_CLASS(NvBlastFramework);
	
	framework = memnew(NvBlastFramework); // Creates the singleton globally
    Engine::get_singleton()->register_singleton("BlastFramework", NvBlastFramework::get_singleton());
}

void uninitialize_example_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	
	Engine::get_singleton()->unregister_singleton("BlastFramework");
	memdelete(framework);
}

extern "C" {
// Initialization.
GDExtensionBool GDE_EXPORT gdblast_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(initialize_example_module);
	init_obj.register_terminator(uninitialize_example_module);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}