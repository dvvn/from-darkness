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
        if (data_.empty())
            return;
        std::ranges::for_each(data_, bind_back(invoker(), msg));
    }

    void default_logs_handler::operator()(const wstring_view msg) const
    {
        if (data_.empty())
            return;
        std::ranges::for_each(data_, bind_back(invoker(), msg));
    }

    basic_logs_handler* logger;
} // namespace fd