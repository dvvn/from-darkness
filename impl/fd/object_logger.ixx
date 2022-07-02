module;

#include <functional>
#include <source_location>
#include <string>

export module fd.object_logger;
import fd.type_name;
import fd.logger;

class object_logger
{
    std::string_view object_name_;

  public:
    object_logger(const std::string_view object_name)
        : object_name_(object_name)
    {
        std::invoke(*this, "created");
    }

    object_logger(const std::source_location location = std::source_location::current())
    {
        //??? obj<??>::obj(
        const std::string_view owner = location.function_name();
        const auto end               = owner.rfind('(');
        const auto start             = owner.substr(0, end).rfind(':') + 1;

        const std::string_view fn_name = owner.substr(start, end);
        const auto real_start          = owner.find(fn_name);
        object_name_                   = owner.substr(real_start, end);
    }

    template <class T>
    object_logger(const std::in_place_type_t<T>)
        : object_logger(fd::type_name<T>())
    {
    }

    ~object_logger()
    {
        std::invoke(*this, "destroyed");
    }

    /* std::string_view object_name() const
  {
      return object_name_;
  } */

    template <typename Str, typename... Args>
    void operator()(const Str& str, Args&&... args) const
    {
        std::invoke(
            fd::logger,
            [&] {
                using char_t = std::remove_cvref_t<decltype(str[0])>;

                const std::basic_string_view<char_t> strv = str;
                std::basic_string<char_t> buffer;

                buffer.reserve(object_name_.size() + 2 + strv.size());
                buffer.append(object_name_.begin(), object_name_.end());
                buffer += static_cast<char_t>(':');
                buffer += static_cast<char_t>(' ');
                buffer.append(strv);

                return buffer;
            },
            std::forward<Args>(args)...);
    }
};

export namespace fd
{
    using ::object_logger;
}