module;

#include <string>
#include <variant>

export module fd.string_or_view;

template <typename T>
size_t small_string_size()
{
#ifdef __cpp_lib_constexpr_string
    constinit auto val = std::basic_string<T>().capacity();
    return val;
#else
    static const std::basic_string<T> dummy;
    return dummy.capacity();
#endif
}

template <typename T>
struct basic_string_or_view
{
    using value_type  = T;
    using string_type = std::basic_string<T>;
    using view_type   = std::basic_string_view<T>;

  private:
    std::variant<string_type, view_type> str_;

  public:
    basic_string_or_view()
    {
        str_.emplace<view_type>();
    }

    basic_string_or_view(const view_type sv)
    {
        str_.emplace<view_type>(sv);
    }

    basic_string_or_view(string_type&& str)
    {
        str_.emplace<string_type>(std::move(str));
    }

    basic_string_or_view(const T* str)
    {
        str_.emplace<view_type>(str);
    }

    basic_string_or_view(string_type& str) = delete;

    operator string_type&() &
    {
        return std::get<string_type>(str_);
    }

    operator string_type() &&
    {
        return std::visit(
            []<class S>(S& ref) -> string_type {
                if constexpr (std::is_same_v<S, string_type>)
                {
                    if (ref.size() > small_string_size<T>())
                        return std::move(ref);

                    // if size < small string size no memory allocated
                }

                return { ref.data(), ref.size() };
            },
            str_);
    }

    operator view_type() const
    {
        return std::visit(
            [](const auto& ref) -> view_type {
                return { ref.data(), ref.size() };
            },
            str_);
    }
};

#define SOV_DECLARE(_TYPE_, _PREFIX_) using _PREFIX_##string_or_view = basic_string_or_view<_TYPE_>;

export namespace fd
{
#ifdef __cpp_lib_char8_t
    SOV_DECLARE(char8_t, u8);
#endif
    SOV_DECLARE(char, );
    SOV_DECLARE(char16_t, w);
    SOV_DECLARE(wchar_t, u16);
    SOV_DECLARE(wchar_t, u32);
} // namespace fd
