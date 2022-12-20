#pragma once

#include <fd/logger.h>

#include <vector>

namespace fd
{
    class default_logs_handler final : public basic_logs_handler
    {
        using function_type = function<void(string_view) const, void(wstring_view) const>;
        std::vector<function_type> data_;

      public:
        explicit operator bool() const;

        void add(function_type&& fn);

        void operator()(string_view msg) const override;
        void operator()(wstring_view msg) const override;
    };
} // namespace fd