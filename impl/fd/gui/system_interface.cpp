module;

#include <fd/core/assert.h>
#include <fd/core/object.h>

#include <RmlUi/Core/SystemInterface.h>

#include <chrono>
#include <format>

module fd.gui.system_interface;
import fd.logger;

using Rml_log = Rml::Log::Type;

template <typename... Args>
static bool _Log(const std::string_view str, Args&&... args)
{
    return std::invoke(
        fd::logger,
        [&] {
            constinit std::string_view prefix = "[RmlUi] ";
            std::string buff;
            buff.reserve(prefix.size() + str.size());
            buff += prefix;
            buff += str;
            return buff;
        },
        std::forward<Args>(args)...);
}

static bool _Log(const Rml_log logtype, const Rml::String& message)
{
    FD_ASSERT(logtype != Rml_log::LT_ALWAYS);
    return _Log("({}) {}", logtype, message);
}

class system_interface_impl final : public Rml::SystemInterface
{
    using clock        = std::chrono::high_resolution_clock;
    using out_duration = std::chrono::duration<double>;
    clock::time_point start_time_;

  public:
    system_interface_impl()
        : start_time_(clock::now())
    {
    }

    double GetElapsedTime() override
    {
        const auto now  = clock::now();
        const auto diff = now - start_time_;
        return duration_cast<out_duration>(diff).count();
    }

    bool LogMessage(Rml_log logtype, const Rml::String& message) override
    {
        switch (logtype)
        {
        case Rml_log::LT_ASSERT: {
            FD_ASSERT(message.c_str());                                // it calls std::terminate if defined
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
            if (_Log(message))
                return true;
            return Rml::SystemInterface::LogMessage(logtype, message);
        case Rml_log::LT_WARNING:
        case Rml_log::LT_INFO:
            _Log(logtype, message);
            return true;
        default:
            FD_ASSERT_UNREACHABLE("Unknown log type detected");
        }
    }

    void SetMouseCursor(const Rml::String& cursor_name) override
    {
        char dummy = 0;
    }
};

FD_OBJECT_BIND_TYPE(system_interface, system_interface_impl);

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
            FD_ASSERT_UNREACHABLE("Unknown log type detected");
        }

        return formatter<std::string_view>::format(str, fc);
    }
};
