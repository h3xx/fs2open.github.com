//
//

#include "engine.h"

#include "scripting/scripting.h"

namespace scripting {
namespace api {

//**********LIBRARY: Engine
ADE_LIB(l_Engine, "Engine", "engine", "Basic engine access functions");

ADE_FUNC(addHook, l_Engine,
		 "string name, function hookFunction, [table conditionals = {}], [function override_func = return false]",
		 "Adds a function to be called from the specified game hook", "boolean",
		 "true if hook was installed properly, false otherwise")
{
	using namespace luacpp;

	const char* hook_name;
	LuaFunction hook;
	LuaTable conditionals;
	LuaFunction override_func;
	if (!ade_get_args(L, "su|tu", &hook_name, &hook, &conditionals, &override_func)) {
		return ADE_RETURN_FALSE;
	}

	if (!hook.isValid()) {
		LuaError(L, "Hook function is invalid!");
		return ADE_RETURN_FALSE;
	}

	ConditionedHook cond_hook;

	script_action action;
	action.action_type = scripting_string_to_action(hook_name);

	if (action.action_type == CHA_NONE) {
		LuaError(L, "Invalid hook name '%s'!", hook_name);
		return ADE_RETURN_FALSE;
	}

	action.hook.hook_function.language = SC_LUA;
	action.hook.hook_function.function = hook;

	if (override_func.isValid()) {
		action.hook.override_function.language = SC_LUA;
		action.hook.override_function.function = override_func;
	}

	cond_hook.AddAction(&action);

	if (conditionals.isValid()) {
		for (const auto& tableEntry : conditionals) {
			const auto& key   = tableEntry.first;
			const auto& value = tableEntry.second;

			if (!key.is(ValueType::STRING)) {
				LuaError(L, "Conditional key '%s' is not a string", key.getValue<SCP_string>().c_str());
				return ADE_RETURN_FALSE;
			}

			if (!value.is(ValueType::STRING)) {
				LuaError(L, "Conditional value '%s' for key '%s' is not a string", value.getValue<SCP_string>().c_str(),
						 key.getValue<SCP_string>().c_str());
				return ADE_RETURN_FALSE;
			}

			script_condition cond;
			cond.condition_type   = scripting_string_to_condition(key.getValue<SCP_string>().c_str());
			cond.condition_string = value.getValue<SCP_string>();

			cond_hook.AddCondition(&cond);
		}
	}

	Script_system.AddConditionedHook(std::move(cond_hook));
	return ADE_RETURN_TRUE;
}

} // namespace api
} // namespace scripting
