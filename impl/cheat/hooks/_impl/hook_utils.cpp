#include "hook_utils.h"

using namespace cheat;
using namespace utl;
using namespace hooks;
using namespace hooks::detail;

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
