module;

#include <string_view>

export module cheat.tools.object_name;
export import nstd.type_name;

template <typename T>
consteval std::string_view drop_type_namespace(const std::string_view drop)
{
    constexpr std::string_view str = nstd::type_name<T>();
    const size_t offset_l = !str.starts_with(drop) ? 0 : drop.size() + 2;
    const size_t offset_r = !std::is_pointer_v<T> ? str.size() : str.find('*');
    return {str.data() + offset_l, str.data() + offset_r};
}

export namespace cheat
{
    template <typename T>
    constexpr std::string_view object_name = drop_type_namespace<T>("cheat");

    /* template <typename T>
    constexpr std::string_view csgo_object_name = drop_type_namespace<T>("cheat::csgo"); */
} // namespace cheat
