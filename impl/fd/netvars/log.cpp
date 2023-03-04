#include <fd/netvars/log.h>
#include <fd/utils/file.h>

#include <glaze/glaze.hpp>

#include <boost/filesystem.hpp>

template <>
struct glz::meta<fd::netvar_info>
{
    using T = fd::netvar_info;

    static constexpr auto value = object("name", &T::name, "offset", &T::offset, "type", [](T& v) { return v.type(); });
};

template <>
struct glz::meta<fd::netvar_table>
{
    using T = fd::netvar_table;

    static constexpr auto name = &T::name;
};

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
    if (file_already_written(fullPath.c_str(), buff_.data(), buff_.size()))
        return;
    write_to_file(fullPath.c_str(), buff_.data(), buff_.size());
}

boost::filesystem::path netvars_log::make_path() const
{
    assert(!dir.empty());
    assert(!file.name.empty());
    assert(file.extension.starts_with('.'));

    return dir / file.name += file.extension;
}

void netvars_log::fill(netvar_table& table)
{
    // assert(buff.empty());

    assert(!dir.empty());
    assert(!file.name.empty());
    assert(file.extension.starts_with('.'));

    glz::write_json(table, buff_);

#if 0
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
        jd::serializer<json_type>(jd::output_adapter(buff), filler).dump(jsRoot, indent > 0, false, indent);
    // buff.shrink_to_fit();
#endif
}

} // namespace fd