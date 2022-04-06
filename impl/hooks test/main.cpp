#include <nstd/runtime_assert.h>

#include <Windows.h>
#include <intrin.h>

#include <array>
#include <string_view>

import dhooks.wrapper;

using namespace dhooks;

#pragma section(".text")
__declspec(allocate(".text")) constexpr std::array<std::uint8_t, 2> gadget{0xFF, 0x23}; // jmp dword ptr[ebx]

#define FWD(_VAL_) std::forward<decltype(_VAL_)>(_VAL_)

#define TARGET_ARGS\
	int test_int, float test_float, std::string_view test_val,std::string_view& test_lref,std::string_view&& test_rref
#define TARGET_ARGS_INST\
	int test_int=123;\
	float test_float=100.123;\
	std::string_view test_val="value";\
	std::string_view test_lref="lref";\
	std::string_view test_rref="rref";
#define TARGET_ARGS_PASS\
	test_int, test_float,test_val,test_lref, static_cast<std::string_view&&>(test_rref)
#define DUMMY_NAME NSTD_CONCAT(dummy,__LINE__)
#define DUMMY_VAR\
	[[maybe_unused]]\
	volatile const char DUMMY_NAME = 1;

template<typename ...T>
static void _Gap(T&&... args)
{
	DUMMY_VAR;
}

#define GAP(...) _Gap(__FUNCTION__,_ReturnAddress(),_AddressOfReturnAddress(),##__VA_ARGS__)

#define STRUCT_NAME(_CALL_CVS_) NSTD_CONCAT(target_struct_,_CALL_CVS_)
#define STRUCT_INSTANCE(_CALL_CVS_) NSTD_CONCAT(STRUCT_NAME(_CALL_CVS_),_inst)
#define FUNC_NAME(_CALL_CVS_) NSTD_CONCAT(target_func_,_CALL_CVS_)
#define HOOK_NAME(_BASE_) NSTD_CONCAT(_BASE_,_hooked)

#define FUNC_BODY\
	GAP(TARGET_ARGS_PASS);\
	return _ReturnAddress( );

#define CALLBACK_FN\
	void callback(TARGET_ARGS) override { GAP(TARGET_ARGS_PASS); }

#define FUNC_IMPL(_CALL_CVS_)\
void* __##_CALL_CVS_ FUNC_NAME(_CALL_CVS_)(TARGET_ARGS) { FUNC_BODY; }

#define PREPARE_STRUCT(_CALL_CVS_)\
struct STRUCT_NAME(_CALL_CVS_)\
{\
	void* __##_CALL_CVS_ target_func(TARGET_ARGS) { FUNC_BODY; }\
};\
struct HOOK_NAME(STRUCT_NAME(_CALL_CVS_)) : select_hook_holder<decltype(&STRUCT_NAME(_CALL_CVS_)::target_func)>\
{\
	HOOK_NAME(STRUCT_NAME(_CALL_CVS_))##( )\
	{\
		GAP( );\
		this->set_target_method(&STRUCT_NAME(_CALL_CVS_)::target_func);\
	}\
	CALLBACK_FN;\
};

#define PREPARE_FUNC(_CALL_CVS_)\
static void* __##_CALL_CVS_ FUNC_NAME(_CALL_CVS_)(TARGET_ARGS)\
{\
	GAP(TARGET_ARGS_PASS);\
	return _ReturnAddress( );\
}\
struct HOOK_NAME(FUNC_NAME(_CALL_CVS_)) : select_hook_holder<decltype(&FUNC_NAME(_CALL_CVS_))>\
{\
	HOOK_NAME(FUNC_NAME(_CALL_CVS_))##( )\
	{\
		GAP( );\
		this->set_target_method(&FUNC_NAME(_CALL_CVS_));\
	}\
	CALLBACK_FN;\
};

#define PREPARE_DATA(_CALL_CVS_)\
	PREPARE_STRUCT(_CALL_CVS_);\
	PREPARE_FUNC(_CALL_CVS_);

template<class H>
static auto make_hook( )
{
	auto obj = std::make_unique<H>( );
	auto h = obj->hook( );
	runtime_assert(h == true);
	auto e = obj->enable( );
	runtime_assert(e == true);
	return obj;
}

template<typename Fn, class H, typename ...Args>
static auto test_hook(Fn fn, H& hook, Args&&...args)
{
	//hooked call
	auto a = std::invoke(fn, FWD(args)...);
	hook->disable( );
	//real call
	auto b = std::invoke(fn, FWD(args)...);

	DUMMY_VAR;
}

template<class H, typename Fn, typename ...Args>
static void simulate_hook(Fn fn, Args&&...args)
{
	auto hook = make_hook<H>( );
	test_hook(fn, hook, FWD(args)...);
}

#define RUN_STRUCT(_CALL_CVS_)\
	STRUCT_NAME(_CALL_CVS_) STRUCT_INSTANCE(_CALL_CVS_);\
	simulate_hook<HOOK_NAME(STRUCT_NAME(_CALL_CVS_))>( [&]{ return STRUCT_INSTANCE(_CALL_CVS_).target_func(TARGET_ARGS_PASS); } );
#define RUN_FUNC(_CALL_CVS_)\
	simulate_hook<HOOK_NAME(FUNC_NAME(_CALL_CVS_))>( [&]{ return std::invoke(FUNC_NAME(_CALL_CVS_),TARGET_ARGS_PASS); } );

#define RUN_DATA(_CALL_CVS_)\
	RUN_STRUCT(_CALL_CVS_);\
	RUN_FUNC(_CALL_CVS_);

#undef fastcall
#undef thiscall
#undef cdecl
#undef stdcall
#undef vectorcall

PREPARE_DATA(fastcall);
PREPARE_STRUCT(thiscall);
PREPARE_DATA(cdecl);
PREPARE_DATA(stdcall);
PREPARE_DATA(vectorcall);

int main(int, char**)
{
	TARGET_ARGS_INST;

	RUN_DATA(fastcall);
	RUN_STRUCT(thiscall);
	RUN_DATA(cdecl);
	RUN_DATA(stdcall);
	RUN_DATA(vectorcall);

	return 0;
}
