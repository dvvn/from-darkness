module;

//#include <typeinfo>

module cheat.gui2.type_info;

using namespace cheat;

gui2::type_info::~type_info( ) = default;

bool gui2::type_info::operator==(const type_info & other) const noexcept
{
	return this->type( ) == other.type( );
}
