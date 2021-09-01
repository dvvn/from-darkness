#include "hook_utils.h"

#ifdef _DEBUG
#include <intrin.h>
#endif

using namespace dhooks;
using namespace dhooks::detail;

static std::shared_ptr<context_shared> _Init_context( )
{
	auto ptr = std::make_shared<context_shared>( );
	ptr->ctx = std::make_unique<context>( );
	return ptr;
}

void hook_holder_data::init( )
{
	static auto hooks_context = _Init_context( );
	context                   = hooks_context;
}

void detail::_Call_fn_trap(call_conversion original, call_conversion called)
{
#ifdef _DEBUG
	[[maybe_unused]] const auto a = _ReturnAddress( );
	[[maybe_unused]] const auto b = _AddressOfReturnAddress( );
	constexpr auto              _ = 0;
#endif // _DEBUG
}

void detail::_Call_fn_trap(call_conversion original)
{
	_Call_fn_trap(original, original);
}
