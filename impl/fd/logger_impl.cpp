#include <fd/logger_impl.h>

#include <algorithm>
#include <utility>

namespace fd
{
    default_logs_handler::operator bool() const
    {
        return data_.empty();
    }

    void default_logs_handler::add(function_type fn)
    {
        data_.emplace_back(std::move(fn));
    }

    void default_logs_handler::operator()(const string_view msg) const
    {
#ifdef _DEBUG
        if (data_.empty())
            return;
#endif
        std::ranges::for_each(data_, bind_back(Invoker, msg));
    }

    void default_logs_handler::operator()(const wstring_view msg) const
    {
#ifdef _DEBUG
        if (data_.empty())
            return;
#endif
        std::ranges::for_each(data_, bind_back(Invoker, msg));
    }

    basic_logs_handler* Logger;
} // namespace fd