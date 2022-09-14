module;

#include <utility>

export module fd.logger.impl;
export import fd.logger;
import fd.callback;

export namespace fd
{
    class default_logs_handler : public basic_logs_handler
    {
        callback_simple_custom<function<void(string_view) const, void(wstring_view) const>> data_;

      public:
        explicit operator bool() const
        {
            return data_.empty();
        }

        template <typename Fn>
        void add(Fn&& fn)
        {
            data_.emplace_back(std::forward<Fn>(fn));
        }

        void operator()(const string_view msg) const override
        {
            if (data_.empty())
                return;
            invoke(data_, msg);
        }

        void operator()(const wstring_view msg) const override
        {
            if (data_.empty())
                return;
            invoke(data_, msg);
        }
    };
} // namespace fd
