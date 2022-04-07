#pragma once

#include <nstd/runtime_assert.h>

#include <functional>
#include <string_view>

//#define CHEAT_HOOK_NAME(_CLASS_,_TYPE_) \
//std::string_view console::object_message_impl<_TYPE_##_base>::get_name( ) const\
//{\
//	return "hooks::"#_CLASS_##"::"#_TYPE_;\
//}
//#define CHEAT_HOOK_INSTANCE(_CLASS_,_TYPE_) \
//CHEAT_HOOK_NAME(_CLASS_,_TYPE_);\
//static nstd::one_instance_obj<_TYPE_##_impl> _TYPE_##_inst;\
//bool _CLASS_::_TYPE_::start( )\
//{\
//	return _TYPE_##_inst->hook( ) && _TYPE_##_inst->enable( );\
//}\
//bool _CLASS_::_TYPE_::stop( )\
//{\
//	return _TYPE_##_inst->disable( );\
//}

template<typename Fn, typename T>
Fn _Get_original_method(T& entry)
{
	Fn ret;
	auto orig = entry.get_original_method( );
	reinterpret_cast<void*&>(ret) = orig;
	return ret;
}

#define CHEAT_HOOK_CALL(_FN_,...)\
	std::invoke(_FN_, __VA_ARGS__)

#define CHEAT_HOOK_CALL_ORIGINAL_EX(_FN_TYPE_,...)\
	CHEAT_HOOK_CALL(_Get_original_method<_FN_TYPE_>(*hook_entry), __VA_ARGS__)

#define CHEAT_HOOK_CALL_ORIGINAL(...)\
	CHEAT_HOOK_CALL_ORIGINAL_EX(decltype(&replace::fn), __VA_ARGS__)

#define CHEAT_HOOK_CALL_ORIGINAL_STATIC(...)\
	CHEAT_HOOK_CALL_ORIGINAL(##__VA_ARGS__)

#define CHEAT_HOOK_CALL_ORIGINAL_MEMBER(...)\
	CHEAT_HOOK_CALL_ORIGINAL(this,##__VA_ARGS__)

#define CHEAT_HOOK_PROCESS_FUNC_IMPL(_FN_NAME_)\
	const auto _FN_NAME_##ed = hook_entry->_FN_NAME_( );\
	console::object_message<hook_entry_t>(_FN_NAME_##ed ? #_FN_NAME_##"ed" : "not "#_FN_NAME_##"ed");

#define CHEAT_HOOK_PROCESS_FUNC(_FN_NAME_)\
	CHEAT_HOOK_PROCESS_FUNC_IMPL(_FN_NAME_);\
	if(!_FN_NAME_##ed) return false;

#define CHEAT_HOOK_PROCESS_FUNC_FINAL(_FN_NAME_)\
	CHEAT_HOOK_PROCESS_FUNC_IMPL(_FN_NAME_);\
	return _FN_NAME_##ed;	

#define CHEAT_HOOK_INSTANCE(_CLASS_,_TYPE_) \
struct hook_entry_##_CLASS_##_##_TYPE_ : dhooks::hook_entry {};\
using hook_entry_t = hook_entry_##_CLASS_##_##_TYPE_;\
std::string_view console::object_message_impl<hook_entry_t>::get_name( ) const\
{\
	return "hooks::"#_CLASS_##"::"#_TYPE_;\
}\
static nstd::one_instance_obj<hook_entry_t> hook_entry;

#define CHEAT_HOOK_INIT(_CLASS_,_TYPE_) \
bool _CLASS_::_TYPE_::start( )\
{\
	if(!hook_entry->created( ))\
	{\
		hook_entry->set_target_method(target( ));\
		hook_entry->set_replace_method(&replace::fn);\
		CHEAT_HOOK_PROCESS_FUNC(create);\
	}\
	CHEAT_HOOK_PROCESS_FUNC_FINAL(enable);\
}\
bool _CLASS_::_TYPE_::stop( )\
{\
	if(!hook_entry.initialized())\
	{\
		console::object_message<hook_entry_t>("not initialized");\
		return false;\
	}\
	CHEAT_HOOK_PROCESS_FUNC_FINAL(disable);\
}

