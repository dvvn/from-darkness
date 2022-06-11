module;

#include <eventpp/callbacklist.h>

export module fds.event;
import fds.event.abstract;

struct policies
{
    using Threading = eventpp::SingleThreading;
};

using fds::abstract_event;

template <typename... Args>
class event : public abstract_event<Args...>
{
    using _AbstractEvent = abstract_event<Args...>;
    using _Event         = eventpp::CallbackList<void(Args...), policies>;

    _Event ev_;

  public:
    using callback_type = /* _Event::Callback */ _AbstractEvent::callback_type;

    /* using handle_type   = _Event::Handle_;
    using mutex_type    = _Event::Mutex; */

    event()
    {
        static_assert(std::same_as<_Event::Callback, callback_type>);
    }

    /* handle_type append(const callback_type& callback)
    {
        return ev_.append(callback);
    }
    */

    /* handle_type operator+=(const callback_type& callback)
    {
        return ev_.append(callback);
    } */

    void append(const callback_type& callback) override
    {
        ev_.append(callback);
    }

    void invoke(Args... args) const override
    {
        ev_(args...);
    }

    bool empty() const override
    {
        return ev_.empty();
    }
};

export namespace fds
{
    using ::event;
}
