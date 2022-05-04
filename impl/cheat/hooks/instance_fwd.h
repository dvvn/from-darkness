#pragma once

#define CHEAT_HOOK_INSTANCE_FWD\
	bool start( ) noexcept;\
	bool stop( ) noexcept;

#define CHEAT_HOOK_MODULE(_CLASS_NAME_,_FN_NAME_) cheat.hooks._CLASS_NAME_._FN_NAME_
#define CHEAT_HOOK_NAMESPACE(_CLASS_NAME_,_FN_NAME_) namespace cheat::hooks::inline _CLASS_NAME_::_FN_NAME_

#define CHEAT_HOOK_INSTANCE_FWD_FULL(_CLASS_NAME_,_FN_NAME_)\
CHEAT_HOOK_NAMESPACE(_CLASS_NAME_,_FN_NAME_)\
{\
	CHEAT_HOOK_INSTANCE_FWD;\
}
