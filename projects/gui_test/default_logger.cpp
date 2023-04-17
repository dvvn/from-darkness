#include <fd/logging/default.h>
#include <fd/logging/init.h>
#include <fd/logging/system_console.h>

#include <fcntl.h>
#include <io.h>

namespace fd
{
// template <typename It>
// static void remove_null_terminator(It it)
//{
//     auto &buff = fmt::detail::get_buffer<It::container_type>(it);
//     auto c     = *std::rbegin(fmt::detail::get_buffer<It::container_type>(it));
//     if (c == 0)
//         buff.resize(buff.size() - 1);
// }

static class : public default_logger_t, system_console
{
    using empty_t = std::true_type;

    int current_mode_ = -1;
    int prev_mode_;

    void set_mode(int mode)
    {
        if (current_mode_ != mode)
        {
            prev_mode_    = _setmode(_fileno(stdout), mode);
            current_mode_ = mode;
        }
    }

    void reset_mode()
    {
        set_mode(prev_mode_);
    }

    void reset_mode_instant()
    {
        current_mode_ = _setmode(_fileno(stdout), prev_mode_);
    }

    [[no_unique_address]] empty_t construct_ = [&] {
        set_mode(_O_BINARY);
        reset_mode_instant();
        add_logger(this);
        return empty_t();
    }();

  public:
    using default_logger_t::write;

    void init() override
    {
        write<log_level::info>("Initialized");
    }

  protected:
    void write(pointer msg, size_t length) override
    {
        set_mode(/*_O_BINARY*/ _O_TEXT);
        system_console::write(msg, length);
    }

    void write(wpointer msg, size_t length) override
    {
        set_mode(_O_U16TEXT);
        system_console::write(msg, length);
    }

} debug_console;

default_logger_p default_logger = &debug_console;
} // namespace fd