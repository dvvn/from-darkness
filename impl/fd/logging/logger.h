#pragma once

#include <fd/logging/core.h>

#include <fmt/format.h>
#include <fmt/xchar.h>

namespace fd
{
struct log_builder : virtual abstract_logger
{
    using itr  = std::back_insert_iterator<fmt::memory_buffer>;
    using witr = std::back_insert_iterator<fmt::wmemory_buffer>;

    virtual void write_before(itr it) const  = 0;
    virtual void write_before(witr it) const = 0;

    virtual void write_after(itr it) const  = 0;
    virtual void write_after(witr it) const = 0;

    template <typename C, typename ...Args>
    void write(fmt::basic_string_view<C> fmt, Args &&...args)
    {
        using buff_t     = fmt::basic_memory_buffer<C>;
        using fmt_args_t = fmt::format_arg_store<fmt::buffer_context<C>, std::remove_cvref_t<Args>>();

        buff_t buff;
        auto it = std::back_inserter(buff);

        write_before(it);
        fmt::vformat_to(it, fmt, fmt_args_t(std::forward<Args>(args)...));
        write_after(it);

        write(buff.data(), buff.size());
    }
};

template <log_level Level>
struct logger : basic_logger<Level>, protected virtual log_builder
{
    template <log_level CurrLevel, typename... FmtArgs>
    void write(fmt::format_string<FmtArgs...> fmt, FmtArgs &&...fmt_args) requires(CurrLevel <= Level)
    {
        static_assert(CurrLevel > log_level::off);
        log_builder::write(fmt.get(), std::forward<FmtArgs>(fmt_args)...);
    }

    template <log_level CurrLevel, typename... FmtArgs>
    void write(fmt::wformat_string<FmtArgs...> fmt, FmtArgs &&...fmt_args) requires(CurrLevel <= Level)
    {
        static_assert(CurrLevel > log_level::off);
        log_builder::write(fmt.get(), std::forward<FmtArgs>(fmt_args)...);
    }
};

template <>
struct logger<log_level::off> : basic_logger<log_level::off>
{
};

//template <log_level Level, log_level... Levels>
//struct logger
//{
//};
//
//struct cum:logger<
//{
//    
//};

} // namespace fd