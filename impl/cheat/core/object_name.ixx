module;

#include <string_view>

export module cheat.tools.object_name;
export import nstd.type_name;

template <typename T>
consteval std::string_view drop_type_namespace(const std::string_view drop)
{
    const std::string_view str = nstd::type_name<T>();
    const size_t offset_l = !str.starts_with(drop) ? 0 : drop.size() + 2;
    const size_t offset_r = !std::is_pointer_v<T> ? 0 : str.size() - str.rfind('*');
    return {str.begin() + offset_l, str.end() - offset_r};
}

export namespace cheat::tools
{
    template <typename T>
    constexpr std::string_view object_name = drop_type_namespace<T>("cheat");

    template <typename T>
    constexpr std::string_view csgo_object_name = drop_type_namespace<T>("cheat::csgo");
} // namespace cheat::tools
