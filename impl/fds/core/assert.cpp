#include <fds/core/assert_impl.h>
#include <fds/core/object.h>

#ifdef _DEBUG
#include <stdexcept>
#endif
#include <algorithm>
#include <mutex>
#include <numeric>
#include <variant>
#include <vector>
#undef NDEBUG
#include <cassert>

template <typename C>
struct msg_packed
{
    using string_type      = std::basic_string<C>;
    using string_view_type = std::basic_string_view<C>;
    using pointer_type     = const C*;

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

  private:
    std::variant<string_type, string_view_type, pointer_type> data_;
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

static void _Assert(const char* expression, const char* message, const std::source_location& location)
{
#if defined(_MSC_VER)
    _wassert(_Assert_msg<wchar_t>(expression, message), msg_packed<wchar_t>(location.file_name()), location.line());
#elif defined(__GNUC__)
    __assert_fail(_Assert_msg<char>(expression, message), location.file_name(), location.line(), location.function_name());
#else
    TODO
#endif
}

using fds::rt_assert_handler;

struct rt_assert_handler_default final : rt_assert_handler
{
    void handle(const char* expression, const char* message, const std::source_location& location) override
    {
        _Assert(expression, message, location);
    }

    void handle(const char* message, const std::source_location& location) override
    {
        _Assert(nullptr, message, location);
    }
};

class rt_assert_data
{
    std::mutex mtx_;
    std::vector<rt_assert_handler*> storage_;

  public:
    template <bool Lock = true>
    void add(rt_assert_handler* const handler)
    {
        if constexpr (Lock)
            mtx_.lock();

#ifdef _DEBUG
        if (!storage_.empty())
        {
            for (const auto& el : storage_)
            {
                if (el == handler)
                    throw std::logic_error("Handler already added!");
            }
        }
#endif
        storage_.push_back(handler);

        if constexpr (Lock)
            mtx_.unlock();
    }

    void remove(rt_assert_handler* const handler)
    {
        const auto lock = std::scoped_lock(mtx_);
        const auto end  = storage_.end();
        for (auto itr = storage_.begin(); itr != end; ++itr)
        {
            if (handler == *itr)
            {
                storage_.erase(itr);
                break;
            }
        }
    }

    template <typename... Args>
    void handle(const Args&... args)
    {
        const auto lock = std::scoped_lock(mtx_);
        for (const auto& el : storage_)
            el->handle(args...);
    }

    rt_assert_data() = default;
};

FDS_OBJECT(_Rt, rt_assert_data);

void fds::_Rt_assert_add(rt_assert_handler* const handler)
{
    _Rt->add(handler);
}

void fds::_Rt_assert_remove(rt_assert_handler* const handler)
{
    _Rt->remove(handler);
}

void fds::_Rt_assert_invoke(const char* expression, const char* message, const std::source_location& location)
{
    _Rt->handle(expression, message, location);
    std::terminate();
}
