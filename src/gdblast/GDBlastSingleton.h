#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "sdk/toolkit/include/NvBlastTk.h"

using namespace Nv::Blast;
using namespace godot;

class NvBlastFramework : public Object {
	GDCLASS(NvBlastFramework, Object);

	static NvBlastFramework *singleton;

protected:
	static void _bind_methods();

public:
	static NvBlastFramework *get_singleton();

	static TkFramework* get_framework() {
		TkFramework* framework = NvBlastTkFrameworkGet();
		if (framework == nullptr) {
			framework = NvBlastTkFrameworkCreate();
			UtilityFunctions::print("NvBlastFramework created!");
		}
		return framework;
	}

	NvBlastFramework();
	~NvBlastFramework();
};