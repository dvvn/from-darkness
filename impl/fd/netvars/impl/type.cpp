#include "hashed_object.h"
#include "type.h"

#include <fd/mem_scanner.h>

#include <boost/filesystem.hpp>
#include <boost/lambda2.hpp>

#include <fmt/format.h>

#include <algorithm>
#include <fstream>

using fd::_const;
using fd::hashed_object;

template <typename T, typename C>
struct fmt::formatter<hashed_object<T>, C> : formatter<T, C>
{
    auto format(_const<hashed_object<T> &> obj, auto &ctx) const -> decltype(ctx.out())
    {
        return formatter<T, C>::format(get<T>(obj), ctx);
    }
};

namespace fd
{
#if 0
// netvar_type_array* netvar_type_array::get_inner_array() const
//{
//     auto& inner = this->data->data;
//     return std::holds_alternative<netvar_type_array>(inner) ? &std::get<netvar_type_array>(inner) : nullptr;
// }

static void absolute_array_type(std::string &buff, netvar_type_array &arr)
{
    auto inner = arr.inner.get();

    buff.append("std::array<");
    if (std::holds_alternative<netvar_type_array>(inner->data))
        absolute_array_type(buff, std::get<netvar_type_array>(inner->data));
    else
        buff.append(inner->get_type());

    fmt::format_to(std::back_inserter(buff), ", {}>", arr.size);
}

netvar_type_array::netvar_type_array(uint16_t size, netvar_type &&type)
    : size(size)
    , inner(std::make_unique<netvar_type>(std::move(type)))
{
}

void netvar_type_array::fill(bool force)
{
    if (force || type.empty())
    {
        type.clear();
        absolute_array_type(type, *this);
    }
}

// custom_netvar_type_ex netvar_type_array::unwrap()
//{
//     fill();
//     custom_netvar_type_ex out;
//     out.type = std::move(this->type);
//     get_netvar_include(*this, out.includes);
// }

static constexpr struct
{
    template <class T>
    std::string_view operator()(T &val) const
    {
        return val.type;
    }

    std::string_view operator()(std::monostate) const
    {
        std::unreachable();
    }

    std::string_view operator()(netvar_type_array &val) const
    {
        val.fill();
        return val.type;
    }

    std::string_view operator()(netvar_type_array const &val) const
    {
        assert(!val.type.empty());
        return val.type;
    }
} _GetNetvarType;

std::string_view netvar_type::get_type()
{
    return std::visit(_GetNetvarType, data);
}

std::string_view netvar_type::get_type() const
{
    return std::visit(_GetNetvarType, data);
}
#if 0
bool netvar_type::unwrap(custom_netvar_type& type)
{
    if (!std::holds_alternative<netvar_type_array>(data))
        return false;
    auto& self = std::get<netvar_type_array>(data);
    if (std::holds_alternative<netvar_type_array>(self.data->data))
        return false;

    self.fill();
    type.type = std::move(self.type);
    get_netvar_include(*self.data, type.include);
    return true;
}

bool netvar_type::unwrap(custom_netvar_type_ex& type)
{
    if (!std::holds_alternative<netvar_type_array>(data))
        return false;
    auto& self = std::get<netvar_type_array>(data);

    self.fill();
    type.type = std::move(self.type);
    get_netvar_include(*self.data, type.includes);
    return true;
}
#endif
#endif

netvar_type_array::netvar_type_array(std::string_view type, size_t size)
    : type_(fmt::format("std::array<{}, {}>", type, size))
    , size_(size)
{
}

std::string_view netvar_type_array::get() const
{
    return type_;
}

size_t netvar_type_array::size() const
{
    return size_;
}

size_t netvar_type_array_size(basic_netvar_type *type)
{
    auto arr = dynamic_cast<netvar_type_array *>(type);
    return arr ? arr->size() : 0;
}

size_t netvar_type_array_size(netvar_type_array *type)
{
    return type->size();
}

using boost::filesystem::directory_iterator;
using boost::filesystem::file_type;
using boost::filesystem::path;

namespace valve_dir
{
static constexpr std::wstring_view dir_ = BOOST_JOIN(L, BOOST_STRINGIZE(FD_VALVE_DIR));

template <typename C>
static path append(std::basic_string_view<C> to_append)
{
    path::string_type buff;
    buff.reserve(dir_.length() + 1 + to_append.length());
    return path(std::move(buff)).append(to_append);
}

static directory_iterator iterate()
{
    return directory_iterator(dir_);
}

} // namespace valve_dir

struct valve_include
{
    // fd/valve/XXXX.h
    hashed_object<std::string> path;
    size_t name_offset;
    size_t name_size;

    std::vector<size_t> inner;
    std::vector<char> data;

    valve_include(std::wstring_view wpath, size_t name_offset, size_t name_size)
        : path({wpath.begin(), wpath.end()}, get_hash(wpath.substr(name_offset, name_size)))
        , name_offset(name_offset)
        , name_size(name_size)
    {
        std::replace(path.first.begin(), path.first.end(), '\\', '/');
    }

    std::string_view name() const
    {
        return get<std::string_view>(path).substr(name_offset, name_size);
    }

    std::string_view full_name() const
    {
        return get<std::string_view>(path).substr(name_offset);
    }

    bool inside(_const<hashed_object<std::string_view> &> name)
    {
        /* if (!name_hash)
             name_hash = netvar_hash(name);*/

        using namespace boost::lambda2;

        if (std::any_of(inner.begin(), inner.end(), _1 == get_hash(name)))
            return true;

        if (data.empty())
        {
            auto full_path = valve_dir::append(full_name());
            data.reserve(file_size(full_path));
            std::ifstream file;
            file.open(full_path.native(), std::ios::binary | std::ios::in);
            assert(file);
            std::copy<std::istream_iterator<char>>(file, {}, std::back_inserter(data));
        }

        constexpr auto is_valid_char = [](char c) {
            return c == '\t' || c == '\n' || c == ' ';
        };

        auto found = find_bytes(
            data.data(),
            data.data() + data.size(),
            name.first.data(),
            name.first.size(),
            [name_size = name.first.size()](char *ptr, auto *stop_token) {
                if (is_valid_char(ptr[-1]) && is_valid_char(ptr[name_size]))
                    stop_token->stop();
                return true;
            });
        if (found)
            inner.emplace_back(name.second);
        return found;
    }
};

namespace valve_classes
{
std::vector<valve_include> storage_;

void fill()
{
    if (!storage_.empty())
        return;

    for (auto &entry : (valve_dir::iterate()))
    {
        if (entry.status().type() != file_type::regular_file)
            continue;

        auto &file     = entry.path();
        auto filename  = file.filename();
        auto extension = filename.extension();
        if (!extension.native().starts_with(L".h"))
            continue;

        // c:/??/??/
        constexpr auto abs_path_length = std::size(BOOST_STRINGIZE(FD_IMPL_DIR));

        auto full_path       = std::basic_string_view(file.native()).substr(abs_path_length);
        auto filename_offset = full_path.size() - filename.size();

        storage_.emplace_back(full_path, filename_offset, filename.stem().size());
    }
    storage_.shrink_to_fit();
}

static valve_include *find(auto callback)
{
    fill();

    auto bg    = storage_.begin();
    auto ed    = storage_.end();
    auto found = std::find_if(bg, ed, std::ref(callback));
    return found == ed ? nullptr : &*found /*storage_.data() + std::distance(bg, found)*/;
}
} // namespace valve_classes

struct valve_class_data
{
    std::vector<char> data;
    size_t include_hash;

    valve_class_data(valve_include &info)
        : include_hash(get_hash(info.path))
    {
        auto full_path = valve_dir::append(info.full_name());
        data.reserve(file_size(full_path));
        std::ifstream file;
        file.open(full_path.native(), std::ios::binary | std::ios::in);
        assert(file);
        std::copy<std::istream_iterator<char>>(file, {}, std::back_inserter(data));
    }
};

void netvar_type_includes(std::string_view type, std::vector<std::string> &buff)
{
    if (auto offset = type.find("std::"); offset != type.npos)
    {
        auto name = type.substr(offset + 1, type.find('<', offset));
        buff.emplace_back(fmt::format("<{}>", name));
    }
    if (auto offset = type.find("valve::"); offset != type.npos)
    {
        hashed_object name = type.substr(offset + 1, type.find('<', offset));

        using namespace boost::lambda2;

        auto target = valve_classes::find(_1->*&valve_include::path == name);

        // if (!target )
        //     target = valve_classes::find([=](valve_include &inc) { return name.contains(inc.name()); });
        if (!target)
            target = valve_classes::find(std::bind_back(&valve_include::inside, name));

        buff.emplace_back(fmt::format("<{}>", target->path));
    }

    if (buff.empty() && type.contains('_'))
    {
        buff.emplace_back("<cstdint>");
    }
}
} // namespace fd