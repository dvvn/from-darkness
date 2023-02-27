#include <fd/netvars/log.h>
#include <fd/utils/file.h>

#include <nlohmann/json.hpp>
#include <nlohmann/ordered_map.hpp>

#include <boost/filesystem.hpp>

namespace fd
{
netvars_log::netvars_log() = default;

netvars_log::~netvars_log()
{
    if (dir.empty())
        return;
    if (!exists(dir) && !create_directories(dir))
        return;
    auto fullPath = make_path();
    if (file_already_written(fullPath.c_str(), buff.data(), buff.size()))
        return;
    write_to_file(fullPath.c_str(), buff.data(), buff.size());
}

boost::filesystem::path netvars_log::make_path() const
{
    assert(!dir.empty());
    assert(!file.name.empty());
    assert(file.extension.starts_with('.'));

    return dir / file.name += file.extension;
}

size_t netvars_log::fill(fill_fn const& updater)
{
    assert(buff.empty());

    assert(!dir.empty());
    assert(!file.name.empty());
    assert(file.extension.starts_with('.'));

    using json_type = nlohmann::ordered_json;

    json_type  jsRoot;
    json_type* jsCurr;

    basic_netvar_table::for_each_fn writer = [&jsCurr](basic_netvar_info const& info)
    {
        auto name   = info.name();
        auto type   = info.type();
        auto offset = info.offset();

        if (type.empty())
        {
            jsCurr->push_back({
                {  "name",   name},
                {"offset", offset}
            });
        }
        else
        {
            jsCurr->push_back({
                {  "name", (name)},
                {"offset", offset},
                {  "type", (type)}
            });
        }
    };

    for (basic_netvar_table const* table; updater(table);)
    {
        assert(!table->empty());

        jsCurr = &jsRoot[table->name()];
        table->for_each(writer);
    }

    namespace jd = nlohmann::detail; // NOLINT(misc-unused-alias-decls)
    buff.reserve(1024 * 220);        // filse size ~220 kb
    jd::serializer<json_type>(jd::output_adapter(buff), filler).dump(jsRoot, indent > 0, false, indent);
    // buff.shrink_to_fit();

    return buff.size();
}

} // namespace fd