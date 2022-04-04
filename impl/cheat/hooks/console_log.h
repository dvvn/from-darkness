#pragma once

#define CHEAT_HOOKS_CONSOLE_LOG_IMPL(_TYPE_,_FN_NAME_)\
bool _TYPE_::_FN_NAME_( )\
{\
	const auto ret = hook_holder_data::_FN_NAME_( );\
	console::object_message<_TYPE_>(ret ? #_FN_NAME_##"ed" : "not "#_FN_NAME_##"ed");\
	return ret;\
}\

#define CHEAT_HOOKS_CONSOLE_LOG(_TYPE_)\
	CHEAT_HOOKS_CONSOLE_LOG_IMPL(_TYPE_, hook);\
	CHEAT_HOOKS_CONSOLE_LOG_IMPL(_TYPE_, enable);\
	CHEAT_HOOKS_CONSOLE_LOG_IMPL(_TYPE_, disable);\
