module;

#include <fd/callback_impl.h>

#include <Windows.h>

#include <exception>
#include <source_location>

module fd.assert;
import fd.string.make;
import fd.utf_convert;
import fd.application_data;
import fd.mutex;

using namespace fd;

template <typename... T>
static auto _Build_message(const assert_data& data, T... extra)
{
#define FIRST_PART "Assertion falied!", '\n', /**/ "File: ", location.file_name(), '\n', /**/ "Line: ", to_string(location.line()), "\n\n"
#define EXPR       "Expression: ", expression
    const auto [expression, message, location] = data;
    string msg;

    if (expression && message)
        msg = make_string(FIRST_PART, EXPR, "\n\n", message, extra...);
    else if (expression)
        msg = make_string(FIRST_PART, EXPR, extra...);
    else if (message)
        msg = make_string(FIRST_PART, message, extra...);
    else
        msg = make_string(FIRST_PART, extra...);

    return utf_convert<wchar_t>(msg);
}

template <typename B>
static auto _Assert_msg(const B builder, const char* expression, const char* message)
{
    if (!expression)
        return builder(message);
    if (!message)
        return builder(expression);

    return builder(expression, " (", message, ')');
}

using assert_handler_base = FD_CALLBACK_TYPE(assert_handler);

template <typename It>
static auto _Swap_with_next(It it)
{
    using std::swap;
    swap(*it, *(it + 1));
}

static void _Default_assert_handler(const assert_data& data)
{
    const auto [expression, message, location] = data;
#ifdef WINAPI_FAMILY
#if WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP
    const auto msg       = _Build_message(data, "\n\nWould you like to interrupt execution?");
    const auto terminate = MessageBoxW(app_info->root_window.handle, msg.c_str(), L"Assertion Failure", MB_YESNO | MB_ICONSTOP | MB_DEFBUTTON2 | MB_TASKMODAL) != IDNO;
    if (terminate)
        std::terminate(); // todo: unload by own function instead of terminate
#else
#pragma error not implemented
#endif

#else
#pragma error not implemented
#endif
}

//#define MANUAL_ASSERT_LOCK

class assert_handler_impl final : public assert_handler_base
{
    mutex mtx_;

  public:
    using assert_handler_base::callback_type;

    assert_handler_impl()
    {
#ifdef MANUAL_ASSERT_LOCK
        assert_handler_base::push_front([this](auto&) {
            mtx_.lock();
        });

        assert_handler_base::push_back([this](auto& data) {
            _Default_assert_handler(data);
            mtx_.unlock();
        });
#else
        assert_handler_base::push_back(_Default_assert_handler);
#endif
    }

#ifdef MANUAL_ASSERT_LOCK
    void push_front(callback_type&& callback) override
    {
        assert_handler_base::push_front(std::move(callback));
        // mutex locker must be first
        _Swap_with_next(this->storage_.begin());
    }
#else
    void operator()(const assert_data& data) override
    {
        const lock_guard l(mtx_);
        invoke(*static_cast<assert_handler_base*>(this), data);
    }
#endif

    void push_back(callback_type&& callback) override
    {
        assert_handler_base::push_back(std::move(callback));
        // messagebox handler must be last
        _Swap_with_next(this->storage_.rbegin());
    }
};

FD_CALLBACK_BIND(assert_handler, assert_handler_impl);

wstring assert_data::build_message() const
{
    return _Build_message(*this);
}
