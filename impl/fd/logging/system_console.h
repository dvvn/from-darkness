#pragma once
#include <fd/logging/logger.h>

namespace fd
{
template <typename C>
struct system_console_base : virtual abstract_logger<C>, virtual internal_logger<C>
{
};

template <typename C>
struct system_console : system_console_base<C>
{
    using typename system_console_base<C>::pointer;
    using typename system_console_base<C>::data_type;

  protected:
    void do_write(pointer msg, size_t length) final
    {
        std::unreachable();
    }

    void write_before(data_type *d) final
    {
        std::unreachable();
    }

    void write_after(data_type *d) final
    {
        std::unreachable();
    }
};

template <>
struct system_console<char> : system_console_base<char>
{
  protected:
    void do_write(pointer msg, size_t length) override;
    void write_before(data_type *d) final;
    void write_after(data_type *d) final;
};

template <>
struct system_console<wchar_t> : system_console_base<wchar_t>
{
  protected:
    void do_write(pointer msg, size_t length) override;
    void write_before(data_type *d) final;
    void write_after(data_type *d) final;
};

template <typename... C>
struct system_console_all : system_console<C>...
{
  protected:
    using system_console<C>::do_write...;
    using system_console<C>::write_before...;
    using system_console<C>::write_after...;
};

template <>
struct system_console<void> : system_console_all<char, wchar_t>
{
  protected:
    using system_console_all::do_write;
    using system_console_all::write_after;
    using system_console_all::write_before;
};

} // namespace fd