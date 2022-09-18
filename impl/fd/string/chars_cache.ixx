module;

#include <algorithm>

export module fd.chars_cache;
export import fd.string;

export namespace fd
{
    template <typename Chr, size_t Size>
    struct chars_cache
    {
        Chr _Data[Size];

        using value_type    = Chr;
        using pointer       = Chr*;
        using const_pointer = const Chr*;

        using _View = basic_string_view<Chr>;

        constexpr chars_cache()
            : _Data()
        {
#ifndef _DEBUG
            _Data[Size - 1] = 0;
#endif
        }

        constexpr chars_cache(const_pointer str_source, const size_t string_size = Size)
        {
            std::copy_n(str_source, string_size, _Data);
            std::fill(_Data + string_size, _Data + Size, static_cast<Chr>(0));
            //_Data[string_size] = 0;
        }

        constexpr const_pointer data() const
        {
            return _Data;
        }

        constexpr pointer data()
        {
            return _Data;
        }

        constexpr size_t size() const
        {
            return Size - 1;
        }

        constexpr const_pointer begin() const
        {
            return _Data;
        }

        constexpr const_pointer end() const
        {
            return _Data + size();
        }

        constexpr pointer begin()
        {
            return _Data;
        }

        constexpr pointer end()
        {
            return _Data + size();
        }

        constexpr _View view() const
        {
            return { begin(), size() };
        }

        constexpr operator _View() const
        {
            return view();
        }
    };

    template <typename Chr, size_t Size>
    chars_cache(const Chr (&arr)[Size]) -> chars_cache<Chr, Size>;
} // namespace fd
