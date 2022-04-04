module;

#include <nstd/runtime_assert.h>

#include <string>
#include <sstream>

module cheat.netvars;
import :storage;
import cheat.console.object_message;
import cheat.csgo.modules;
import cheat.csgo.interfaces.C_BaseEntity;
import cheat.csgo.interfaces.EngineClient;
import cheat.csgo.interfaces.BaseClient;

using namespace cheat;
using namespace netvars;
using namespace csgo;

static std::wstring to_wstring(const char* str, const bool reserve = false)
{
	std::wstring out;
	if (reserve)
	{
		const auto size = std::char_traits<char>::length(str);
		out.reserve(size);
		out.append(str, str + size);
	}
	else
	{
		for (auto c = str[0]; c != '\0'; c = *++str)
			out += static_cast<wchar_t>(c);

	}
	return out;
}

class netvars_holder;
std::string_view console::object_message_impl<netvars_holder>::get_name( ) const
{
	return "netvars";
}

class netvars_holder : public storage, console::object_message_auto<netvars_holder>
{
#ifdef _DEBUG
	logs_data logs;
	classes_data classes;
#endif

public:
	netvars_holder( )
	{
		iterate_client_class(IBaseClientDLL::get( ).GetAllClasses( ));

		const auto baseent = csgo_modules::client.find_vtable<C_BaseEntity>( );
		iterate_datamap(baseent->GetDataDescMap( ));
		iterate_datamap(baseent->GetPredictionDescMap( ));

		store_handmade_netvars( );

#ifdef _DEBUG
		logs.file.name = to_wstring(IVEngineClient::get( ).GetProductVersionString( ));
		log_netvars(logs);
		generate_classes(classes);
#endif 
	}
};

static nstd::one_instance_obj<netvars_holder> holder;

size_t netvars::get_offset(const std::string_view table, const std::string_view item)
{
	/*const auto& storage = nstd::one_instance<netvars_holder>::get( ).storage;

	const auto target_class = find(table);
	runtime_assert(target_class != end( ));
	const auto netvar_info = target_class->find(item);
	runtime_assert(netvar_info != target_class->end( ));

	using namespace std::string_view_literals;
	return netvar_info->find("offset"sv)->get<int>( );*/

	return holder->find(table)->find(item)->offset( );
}

void netvars::construct( )
{
	(void)*holder;
}
