#pragma once

#include <fd/logging/core.h>
#include <fd/logging/data.h>
#include <fd/logging/internal.h>

#include <fmt/format.h>
#include <fmt/xchar.h>

namespace fd
{
template <log_level Level>
struct dummy_logger
{
    template <log_level CurrLevel>
    void write(...) //
        requires(!have_log_level(Level, CurrLevel))
    {
        (void)this;
    }
};

template <log_level Level, typename C>
struct basic_logger : protected virtual abstract_logger<C>, protected virtual internal_logger<C>
{
    template <log_level CurrLevel, typename... FmtArgs>
    void write(fmt::basic_format_string<C, std::type_identity_t<FmtArgs>...> fmt, FmtArgs &&...fmt_args)
        requires(have_log_level(Level, CurrLevel))
    {
        using ctx         = fmt::buffer_context<C>;
        using args_stored = fmt::format_arg_store<ctx, std::remove_cvref_t<FmtArgs>...>;

        auto internal = static_cast<internal_logger<C> *>(this);
        auto abstract = static_cast<abstract_logger<C> *>(this);

        auto data = internal_logger<C>::data_type(CurrLevel);

        internal->write_before(&data);
        fmt::vformat_to(data.out(), fmt.get(), args_stored(std::forward<FmtArgs>(fmt_args)...));
        internal->write_after(&data);

        abstract->do_write(data.data(), data.size());
    }
};

template <typename C>
struct logger_info
{
    log_level level;
    using char_type = C;

    constexpr logger_info(log_level level)
        : level(level)
    {
    }

    constexpr logger_info(log_level level, C)
        : logger_info(level)
    {
    }

    constexpr logger_info(log_level level, std::in_place_type_t<C>)
        : logger_info(level)
    {
    }
};

template <typename... C>
struct supported_char_types
{
    template <typename C1>
    static constexpr bool support()
    {
        return (std::is_same_v<C, C1> || ...);
    }

    template <template <typename> class T>
    struct merge : T<C>...
    {
    };
};

template <log_level Level, typename... C>
struct logger_same_level : basic_logger<Level, C>..., dummy_logger<Level>
{
    using char_types = supported_char_types<C...>;

    static constexpr log_level level()
    {
        return Level;
    }

    using basic_logger<Level, C>::write...;
    using dummy_logger<Level>::write;
};

template <logger_info... Info>
struct logger : basic_logger<Info.level, typename decltype(Info)::char_type>..., dummy_logger<(Info.level | ...)>
{
    using char_types = supported_char_types<typename decltype(Info)::char_type...>;

    static constexpr log_level level()
    {
        return (Info.level | ...);
    }

    using basic_logger<Info.level, typename decltype(Info)::char_type>::write...;
    using dummy_logger<level()>::write;
};

// template <typename Logger, typename C>
// concept can_log = std::is_convertible_v<Logger *, basic_logger<Logger::level(), C> *>;

template <class Logger, template <typename C> class Impl>
struct logger_impl_wrapped : Logger, Logger::char_types::template merge<Impl>
{
};

} // namespace fd