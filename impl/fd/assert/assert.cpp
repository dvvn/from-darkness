module;

#include <fd/callback_impl.h>

#include <functional>
#include <source_location>
#include <string>
#include <variant>

module fd.assert;

#undef NDEBUG
#include <assert.h>

using assert_callback          = fd::callback<const assert_data&>;
using assert_callback_ex       = fd::callback_ex<1, const assert_data&>;
using selected_assert_callback = std::conditional_t<(assert_callback::known_buffer_size() > 1), assert_callback, assert_callback_ex>;

struct assert_handler_impl : selected_assert_callback
{
    assert_handler_impl();

    void invoke(const assert_data& data) const override
    {
        selected_assert_callback::invoke(data);
        std::terminate();
    }
};

FD_CALLBACK_BIND(assert_handler, assert_handler_impl);

template <typename C>
struct msg_packed
{
    using string_type      = std::basic_string<C>;
    using string_view_type = std::basic_string_view<C>;
    using pointer_type     = const C*;

    using value_type = std::variant<string_type, string_view_type, pointer_type>;

    msg_packed() = default;

    msg_packed(string_type&& str)
        : data_(std::move(str))
    {
    }

    msg_packed(pointer_type ptr)
        : data_(ptr)
    {
    }

    template <typename T>
    msg_packed(const T* ptr)
    {
        static_assert(sizeof(T) < sizeof(C));
        data_.emplace<string_type>(ptr, ptr + std::char_traits<T>::length(ptr));
    }

    operator pointer_type() const
    {
        return std::visit(
            []<typename T>(const T& obj) {
                if constexpr (std::is_class_v<T>)
                    return obj.data();
                else
                    return obj;
            },
            data_);
    }

    string_view_type view() const
    {
        const auto str = std::visit(
            [](const auto& obj) -> string_view_type {
                return obj;
            },
            data_);
        if (std::holds_alternative<pointer_type>(data_))
            const_cast<value_type&>(data_).emplace<string_view_type>(str);
        return str;
    }

  private:
    value_type data_;
};

template <typename C>
msg_packed(const C*) -> msg_packed<C>;

template <typename C, typename... Args>
static auto _Join(Args... args)
{
    constexpr auto sized = []<typename T>(const T& obj) {
        if constexpr (std::is_class_v<T>)
            return std::pair(obj.data(), obj.size());
        else if constexpr (std::is_pointer_v<T>)
            return std::pair(obj, std::char_traits<std::remove_pointer_t<T>>::length(obj));
        else if constexpr (std::is_bounded_array_v<T>)
            return std::pair(static_cast<std::decay_t<T>>(obj), std::size(obj) - 1);
        else
            return std::pair(obj, 1);
    };

    constexpr auto append = []<class Buff, typename T, typename S>(Buff& buff, const std::pair<T, S> obj) {
        auto [src, size] = obj;
        if constexpr (std::is_pointer_v<T>)
        {
            buff.append(src, src + size);
        }
        else
        {
            do
                buff += src;
            while (--size > 0);
        }
    };

    return std::apply(
        [](auto... pairs) {
            std::basic_string<C> buff;
            buff.reserve((pairs.second + ...));
            (append(buff, pairs), ...);
            return buff;
        },
        std::tuple(sized(args)...));
}

std::string assert_data::build_message() const
{
    const auto part1 = _Join<char>("Assertion falied!", '\n',            /**/
                                   "File: ", location.file_name(), '\n', /**/
                                   "Line: ", std::to_string(location.line()), "\n\n");
    msg_packed<char> part2;
    if (expression && message)
        part2 = _Join<char>("Expression: ", expression, '\n\n', message);
    else if (expression)
        part2 = _Join<char>("Expression: ", expression);
    else if (message)
        part2 = message;

    return _Join<char>(part1, part2.view());
}

template <typename C>
static auto _Assert_msg(const char* expression, const char* message)
{
    msg_packed<C> out;

    if (!expression)
        out = message;
    else if (!message)
        out = expression;
    else
        out = _Join<C>(expression, "( ", message, ')');

    return out;
}

assert_handler_impl::assert_handler_impl()
{
    selected_assert_callback::append([](const assert_data& data) {
        const auto [expression, message, location] = data;
#if defined(_MSC_VER)
        _wassert(_Assert_msg<wchar_t>(expression, message), msg_packed<wchar_t>(location.file_name()), location.line());
#elif defined(__GNUC__)
        __assert_fail(_Assert_msg<char>(expression, message), location.file_name(), location.line(), location.function_name());
#else
#error not implemented
#endif
    });
}
