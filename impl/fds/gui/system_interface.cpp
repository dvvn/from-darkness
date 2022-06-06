module;

#include <fds/core/assert.h>
#include <fds/core/object.h>

#include <RmlUi/Core/SystemInterface.h>

#include <chrono>
#include <format>

module fds.gui.system_interface;
import fds.logger.system_console;

using Rml_log = Rml::Log::Type;

class system_interface_impl final : public Rml::SystemInterface
{
    using clock        = std::chrono::high_resolution_clock;
    using out_duration = std::chrono::duration<double>;
    clock::time_point start_time_;

  public:
    system_interface_impl();

    double GetElapsedTime() override;
    bool LogMessage(Rml_log logtype, const Rml::String& message) override;
};

FDS_OBJECT_BIND(Rml::SystemInterface, system_interface, system_interface_impl);

system_interface_impl::system_interface_impl()
    : start_time_(clock::now())
{
}

double system_interface_impl::GetElapsedTime()
{
    const auto now  = clock::now();
    const auto diff = now - start_time_;
    return duration_cast<out_duration>(diff).count();
}

using fds::logger_system_console;

template <typename... Args>
static void _Log(const std::string_view str, Args&&... args)
{
    if (!logger_system_console->active())
        return;

    constinit std::string_view prefix = "[RmlUi] ";
    std::string buff;
    buff.reserve(prefix.size() + str.size());
    buff += prefix;
    buff += str;
    logger_system_console->log(buff, std::forward<Args>(args)...);
}

static void _Log(const Rml_log logtype, const Rml::String& message)
{
    if (logtype == Rml_log::LT_ALWAYS)
        _Log(message);
    else
        _Log("({}) {}", logtype, message);
}

bool system_interface_impl::LogMessage(Rml_log logtype, const Rml::String& message)
{
    switch (logtype)
    {
    case Rml_log::LT_ASSERT: {
        fds_assert(message.c_str());                               // it calls std::terminate if defined
        return Rml::SystemInterface::LogMessage(logtype, message); // otherwise call default logs handler
    }
    case Rml_log::LT_ERROR:
        _Log(logtype, message);
        return false;
    case Rml_log::LT_DEBUG:
#ifndef _DEBUG
        return true;
#endif
    case Rml_log::LT_ALWAYS:
        if (!logger_system_console->active())
            return Rml::SystemInterface::LogMessage(logtype, message);
    case Rml_log::LT_WARNING:
    case Rml_log::LT_INFO:
        _Log(logtype, message);
        return true;
    default:
        fds_assert_unreachable("Unknown log type detected");
    }
}

template <>
struct std::formatter<Rml_log, char> : formatter<std::string_view>
{
    template <class FormatContext>
    auto format(const Rml_log type, FormatContext& fc) const
    {
        std::string_view str;
        switch (type)
        {
        case Rml_log::LT_ERROR:
            str = "ERROR";
            break;
        case Rml_log::LT_ASSERT:
            str = "FATAL ERROR";
            break;
        case Rml_log::LT_WARNING:
            str = "WARNING";
            break;
        // case Rml_log::LT_ALWAYS:
        case Rml_log::LT_INFO:
            str = "INFO";
            break;
        case Rml_log::LT_DEBUG:
            str = "DEBUG";
            break;
        default:
            fds_assert_unreachable("Unknown log type detected");
        }

        return formatter<std::string_view>::format(str, fc);
    }
};
