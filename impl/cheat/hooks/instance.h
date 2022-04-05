#pragma once

#include <string_view>

#define CHEAT_HOOK_INSTANCE(_CLASS_,_TYPE_) \
std::string_view console::object_message_impl<_TYPE_##_base>::get_name( ) const\
{\
	return "hooks::"#_CLASS_##"::"#_TYPE_;\
}\
static nstd::one_instance_obj<_TYPE_##_impl> _TYPE_##_inst;\
bool _CLASS_::_TYPE_::start( )\
{\
	return _TYPE_##_inst->hook( ) && _TYPE_##_inst->enable( );\
}\
bool _CLASS_::_TYPE_::stop( )\
{\
	return _TYPE_##_inst->disable( );\
}