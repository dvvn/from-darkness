#pragma once

#include <fd/log.h>
//#include <fd/string.h>

#include <source_location>

namespace fd
{
    class object_log
    {
        string_view object_name_;

        static consteval string_view _Fix_fn_name(const string_view owner)
        {
            //??? obj<??>::obj(
            const auto end   = owner.rfind('(');
            const auto start = owner.substr(0, end).rfind(':') + 1;

            const auto fn_name    = owner.substr(start, end);
            const auto real_start = owner.find(fn_name);
            return owner.substr(real_start, end);
        }

      public:
        ~object_log()
        {
            invoke(*this, "destroyed");
        }

        object_log(const string_view object_name = _Fix_fn_name(std::source_location::current().function_name()))
            : object_name_(object_name)
        {
            invoke(*this, "created");
        }

        /* string_view object_name() const
        {
            return object_name_;
        } */

        template <typename Str, typename... Args>
        void operator()(const Str& str, Args&&... args) const
        {
            invoke(log, bind_front(make_string, object_name_, ": ", str), std::forward<Args>(args)...);
        }
    };
} // namespace fd
