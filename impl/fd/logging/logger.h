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

  private:
    template <typename... Args>
    static auto make_format_args(std::type_identity<char>, Args &&...args)
    {
        return fmt::make_format_args(std::forward<Args>(args)...);
    }

    template <typename... Args>
    static auto make_format_args(std::type_identity<wchar_t>, Args &&...args)
    {
        return fmt::make_wformat_args(std::forward<Args>(args)...);
    }

  protected:
    virtual void write_before(itr it) const  = 0;
    virtual void write_before(witr it) const = 0;

    virtual void write_after(itr it) const  = 0;
    virtual void write_after(witr it) const = 0;

    template <typename C, typename... Args>
    void write(fmt::basic_string_view<C> fmt, Args &&...args)
    {
        using buff_t = fmt::basic_memory_buffer<C>;

        buff_t buff;
        auto it = std::back_inserter(buff);

        write_before(it);
        fmt::vformat_to(it, fmt, make_format_args(std::type_identity<C>(), std::forward<Args>(args)...));
        write_after(it);

        static_cast<abstract_logger *>(this)->write(buff.data(), buff.size());
    }
};

template <log_level Level>
struct logger : basic_logger<Level>, protected virtual log_builder
{
    using basic_logger<Level>::write;

    template <log_level CurrLevel, typename... FmtArgs>
    void write(fmt::format_string<FmtArgs...> fmt, FmtArgs &&...fmt_args) requires(CurrLevel &Level)
    {
        log_builder::write(fmt.get(), std::forward<FmtArgs>(fmt_args)...);
    }

    template <log_level CurrLevel, typename... FmtArgs>
    void write(fmt::wformat_string<FmtArgs...> fmt, FmtArgs &&...fmt_args) requires(CurrLevel &Level)
    {
        log_builder::write(fmt.get(), std::forward<FmtArgs>(fmt_args)...);
    }
};

template <>
struct logger<log_level::off> : basic_logger<log_level::off>
{
};

// template <log_level Level, log_level... Levels>
// struct logger
//{
// };
//
// struct cum:logger<
//{
//
// };

} // namespace fd