module;

#include <source_location>
#include <utility>

export module fd.object_logger;
export import fd.logger;
export import fd.string.make;
export import fd.functional.bind;

export namespace fd
{
    class object_logger
    {
        string_view object_name_;

        static consteval string_view _Fix_fn_name(const string_view owner)
        {
            //??? obj<??>::obj(
            const auto end   = owner.rfind('(');
            const auto start = owner.substr(0, end).rfind(':') + 1;

            const auto fn_name    = owner.substr(start, end);
            const auto real_start = owner.find(fn_name);
            return owner.substr(real_start, end);
        }

      public:
        ~object_logger()
        {
            invoke(*this, "destroyed");
        }

        object_logger(const string_view object_name = _Fix_fn_name(std::source_location::current().function_name()))
            : object_name_(object_name)
        {
            invoke(*this, "created");
        }

        /* string_view object_name() const
        {
            return object_name_;
        } */

        template <typename Str, typename... Args>
        void operator()(const Str& str, Args&&... args) const
        {
            invoke(logger, bind_front(make_string, object_name_, ": ", str), std::forward<Args>(args)...);
        }
    };

} // namespace fd
