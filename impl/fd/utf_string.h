#pragma once

#include <fd/string.h>

#include <ww898/utf_converters.hpp>

namespace ww898::utf::detail
{
    template <>
    struct utf_selector<char8_t> final
    {
        using type = utf8;
    };
} // namespace ww898::utf::detail

namespace fd
{
    template <typename T>
    class utf_string : public basic_string<T>
    {
        template <typename It>
        static auto _Convert(It bg, It ed)
        {
            using from = std::iter_value_t<It>;
            using to   = T;

            basic_string<T> buff;

            // std::random_access_iterator false on string::begin wrappers etc
            if constexpr (std::same_as<typename std::iterator_traits<It>::iterator_category, std::random_access_iterator_tag>)
                buff.reserve(std::distance(bg, ed));

            using namespace ww898::utf;
            conv<utf_selector_t<from>, utf_selector_t<to>>(bg, ed, std::back_insert_iterator(buff));

            return buff;
        }

      public:
        using basic_string<T>::basic_string;
        using basic_string<T>::operator=;

        template <typename C>
        utf_string(const basic_string_view<C> from)
            : basic_string<T>(_Convert(from.begin(), from.end()))
        {
        }

        template <typename C>
        utf_string(const basic_string<C>& from)
            : basic_string<T>(_Convert(from.begin(), from.end()))
        {
        }

        template <typename C>
        utf_string(const C* from)
            : basic_string<T>(_Convert<const C*>(from, from + str_len(from)))
        {
        }
    };
} // namespace fd
