#include "GDBlastSingleton.h"

#include <godot_cpp/core/class_db.hpp>


#include "nvblast/sdk/toolkit/source/NvBlastTkFrameworkImpl.h"

using namespace godot;

NvBlastFramework *NvBlastFramework::singleton = nullptr;

void NvBlastFramework::_bind_methods() {
	//ClassDB::bind_method(D_METHOD("get_framework"), &NvBlastFramework::get_framework);
}

NvBlastFramework *NvBlastFramework::get_singleton()
{
	return singleton;
}

NvBlastFramework::NvBlastFramework()
{
	ERR_FAIL_COND(singleton != nullptr); // Should be null, should only be made once!
	singleton = this;
}

NvBlastFramework::~NvBlastFramework()
{
	ERR_FAIL_COND(singleton != this); // Should have been created
	TkFramework* framework = get_framework();
	if (framework != nullptr) {
		framework->release();
	}
	singleton = nullptr;
}