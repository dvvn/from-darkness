module;

#include <nstd/runtime_assert.h>

#include <string>
#include <sstream>

module cheat.netvars.core;
import :storage;
import cheat.console.object_message;
import cheat.csgo.modules;
import cheat.csgo.interfaces.C_BaseEntity;
import cheat.csgo.interfaces.EngineClient;
import cheat.csgo.interfaces.BaseClient;

using namespace cheat;
using namespace netvars;
using namespace csgo;
using namespace console;

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
std::string_view object_message_impl<netvars_holder>::get_name( ) const noexcept
{
	return "netvars";
}

class netvars_holder : public storage
{
	logs_data logs_;
	classes_data classes_;
	[[no_unique_address]]
	object_message_auto<netvars_holder> msg_;

public:
	netvars_holder( )
	{
		iterate_client_class(IBaseClientDLL::get( ).GetAllClasses( ));

		const auto baseent = csgo_modules::client.find_vtable<C_BaseEntity>( );
		iterate_datamap(baseent->GetDataDescMap( ));
		iterate_datamap(baseent->GetPredictionDescMap( ));

		store_handmade_netvars( );
	}

	void log( ) noexcept
	{
		logs_.file.name = to_wstring(IVEngineClient::get( ).GetProductVersionString( ));
		log_netvars(logs_);
		generate_classes(classes_);
	}
};

static nstd::one_instance_obj<netvars_holder> holder;

size_t netvars::get_offset(const std::string_view table, const std::string_view item) noexcept
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

void netvars::write_logs( ) noexcept
{
	holder->log( );
}
