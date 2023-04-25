#include "log.h"

#include <boost/filesystem.hpp>

#include <fstream>

namespace fd
{
netvar_log::netvar_log() = default;

netvar_log::~netvar_log()
{
    if (dir.empty())
        return;
    if (!exists(dir) && !create_directories(dir))
        return;

    std::ofstream f;
    f.rdbuf()->pubsetbuf(nullptr, 0);
    f.open(make_path().native(), std::ios::binary);
    f.write(buff_.data(), buff_.size());
}

boost::filesystem::path netvar_log::make_path() const
{
    assert(!dir.empty());
    assert(!file.name.empty());
    assert(file.extension.starts_with('.'));

    return dir.generic_path().append(file.name).concat(file.extension);
}

void netvar_log::fill(netvar_table &table)
{
    // assert(buff.empty());

    assert(!dir.empty());
    assert(!file.name.empty());
    assert(file.extension.starts_with('.'));

    if (buff_.empty())
        (void)0; // write time or something
    else
        buff_.push_back('\n');
    buff_.append_range(table.name());
    for (auto &v : table)
    {
        // fmt::format_to(std::back_inserter(buff_), "\n\t{} {:#X} {}", v.name(), v.offset(), v.type());
    }

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