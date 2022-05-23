module;

#include <cheat/core/object.h>

#include <nstd/format.h>
#include <nstd/runtime_assert.h>

#include <RmlUi/Core/SystemInterface.h>

#include <chrono>

module cheat.gui.system_interface;
import cheat.logger.system_console;

using Rml_log = Rml::Log::Type;

class system_interface final : public _Sys_ifc
{
    using clock = std::chrono::high_resolution_clock;
    using out_duration = std::chrono::duration<double>;
    clock::time_point start_time_;

  public:
    system_interface();

    double GetElapsedTime() override;
    bool LogMessage(Rml_log logtype, const Rml::String& message) override;
};

CHEAT_OBJECT_BIND(_Sys_ifc, system_interface);

system_interface::system_interface()
    : start_time_(clock::now())
{
}

double system_interface::GetElapsedTime()
{
    const auto now = clock::now();
    const auto diff = now - start_time_;
    return duration_cast<out_duration>(diff).count();
}

template <typename... Args>
static void _Log(const std::string_view str, Args&&... args) noexcept
{
    cheat::logger_system_console->log("[RmlUi] {}", [&] {
        return nstd::vformat(str, nstd::make_format_args(std::forward<Args>(args)...));
    });
}

static void _Log(const Rml_log logtype, const Rml::String& message)
{
    _Log("({}) {}", logtype, message);
}

bool system_interface::LogMessage(Rml_log logtype, const Rml::String& message)
{
    switch (logtype)
    {
    case Rml_log::LT_ASSERT: {
        runtime_assert(message.c_str());               // it calls std::terminate if defined
        return _Sys_ifc::LogMessage(logtype, message); // otherwise call default logs handler
    }
    case Rml_log::LT_ERROR:
        _Log(logtype, message);
        return false;
    case Rml_log::LT_DEBUG:
#ifndef _DEBUG
        return true;
#endif
    case Rml_log::LT_ALWAYS:
        if (!cheat::logger_system_console.initialized() || !cheat::logger_system_console->active())
            return _Sys_ifc::LogMessage(logtype, message);
    case Rml_log::LT_WARNING:
    case Rml_log::LT_INFO:
        _Log(logtype, message);
        return true;
    default:
        runtime_assert_unreachable("Unknown log type detected");
    }
}

template <>
struct nstd::formatter<Rml_log, char> : formatter<std::string_view>
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
        case Rml_log::LT_ALWAYS:
        case Rml_log::LT_INFO:
            str = "INFO";
            break;
        case Rml_log::LT_DEBUG:
            str = "DEBUG";
            break;
        default:
            runtime_assert("Unknown log type detected");
            str = "UNKNOWN";
            break;
        }

        return formatter<std::string_view>::format(str, fc);
    }
};
