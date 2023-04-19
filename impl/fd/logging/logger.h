#pragma once

#include <fd/logging/core.h>
#include <fd/logging/data.h>
#include <fd/logging/internal.h>

#include <fmt/format.h>
#include <fmt/xchar.h>

namespace fd
{
template <typename C, log_level Level>
struct logger : protected virtual abstract_logger<C>, protected virtual internal_logger<C>
{
    using typename internal_logger<C>::data_type;

    template <log_level CurrLevel>
    void write(...) //
        requires(!have_log_level(Level, CurrLevel))
    {
        (void)this;
    }

    template <log_level CurrLevel, typename... FmtArgs>
    void write(fmt::basic_format_string<C, std::type_identity_t<FmtArgs>...> fmt, FmtArgs &&...fmt_args)
        requires(have_log_level(Level, CurrLevel))
    {
        using ctx         = fmt::buffer_context<C>;
        using args_stored = fmt::format_arg_store<ctx, std::remove_cvref_t<FmtArgs>...>;

        auto internal = static_cast<internal_logger<C> *>(this);
        auto abstract = static_cast<abstract_logger<C> *>(this);

        auto data = data_type(CurrLevel);

        internal->write_before(&data);
        fmt::vformat_to(data.out(), fmt.get(), args_stored(std::forward<FmtArgs>(fmt_args)...));
        internal->write_after(&data);

        abstract->do_write(data.data(), data.size());
    }
};
} // namespace fd