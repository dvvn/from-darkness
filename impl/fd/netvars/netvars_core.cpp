module;

#include <fd/core/assert.h>
#include <fd/core/object.h>

#include <sstream>
#include <string>

module fd.netvars.core;
import :storage;
import fd.object_logger;
import fd.rt_modules;
import fd.csgo.interfaces.C_BaseEntity;
import fd.csgo.interfaces.EngineClient;
import fd.csgo.interfaces.BaseClient;

using namespace fd;
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

class netvars_holder : public storage
{
    logs_data logs_;
    classes_data classes_;
    object_logger msg_ = "netvars";

  public:
    netvars_holder()
    {
        iterate_client_class(FD_OBJECT_GET(IBaseClientDLL)->GetAllClasses());

        const auto baseent = runtime_modules::client.find_vtable<C_BaseEntity>();
        iterate_datamap(baseent->GetDataDescMap());
        iterate_datamap(baseent->GetPredictionDescMap());

        store_handmade_netvars();
    }

    void log()
    {
        logs_.file.name = to_wstring(FD_OBJECT_GET(IVEngineClient)->GetProductVersionString());
        log_netvars(logs_);
        generate_classes(classes_);
    }
};

FD_OBJECT(holder, netvars_holder);

size_t netvars::get_offset(const std::string_view table, const std::string_view item)
{
    /*const auto& storage = nstd::one_instance<netvars_holder>::get( ).storage;

    const auto target_class = find(table);
    FD_ASSERT(target_class != end( ));
    const auto netvar_info = target_class->find(item);
    FD_ASSERT(netvar_info != target_class->end( ));

    using namespace std::string_view_literals;
    return netvar_info->find("offset"sv)->get<int>( );*/

    return holder->find(table)->find(item)->offset();
}

void netvars::write_logs()
{
    holder->log();
}
