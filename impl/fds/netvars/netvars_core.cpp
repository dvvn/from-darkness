module;

#include <fds/tools/interface.h>

#include <fds/core/assert.h>

#include <sstream>
#include <string>

module fds.netvars.core;
import :storage;
import fds.console.object_message;
import fds.csgo.modules;
import fds.csgo.interfaces.C_BaseEntity;
import fds.csgo.interfaces.EngineClient;
import fds.csgo.interfaces.BaseClient;

using namespace fds;
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

std::string_view object_message_impl<netvars_holder>::object_name() const
{
    return "netvars";
}

class netvars_holder : public storage
{
    logs_data logs_;
    classes_data classes_;
    [[no_unique_address]] object_message_auto<netvars_holder> msg_;

  public:
    netvars_holder()
    {
        iterate_client_class(nstd::instance_of<IBaseClientDLL*>->GetAllClasses());

        const auto baseent = csgo_modules::client.find_vtable<C_BaseEntity>();
        iterate_datamap(baseent->GetDataDescMap());
        iterate_datamap(baseent->GetPredictionDescMap());

        store_handmade_netvars();
    }

    void log()
    {
        logs_.file.name = to_wstring(nstd::instance_of<IVEngineClient*>->GetProductVersionString());
        log_netvars(logs_);
        generate_classes(classes_);
    }
};

constexpr auto holder = instance_of<netvars_holder>;

size_t netvars::get_offset(const std::string_view table, const std::string_view item)
{
    /*const auto& storage = nstd::one_instance<netvars_holder>::get( ).storage;

    const auto target_class = find(table);
    fds_assert(target_class != end( ));
    const auto netvar_info = target_class->find(item);
    fds_assert(netvar_info != target_class->end( ));

    using namespace std::string_view_literals;
    return netvar_info->find("offset"sv)->get<int>( );*/

    return holder->find(table)->find(item)->offset();
}

void netvars::write_logs()
{
    holder->log();
}
