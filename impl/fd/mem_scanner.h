#pragma once

#include <fd/string.h>

#include <iterator>

namespace fd
{
    struct memory_range
    {
        const uint8_t *from_, *to_;

        memory_range(const uint8_t* from, const uint8_t* to);
        memory_range(const uint8_t* from, const size_t size);

        void update(const uint8_t* curr, const size_t offset = 0);
        void update(void* curr, const size_t offset = 0);
    };

    class unknown_bytes_range;

    class unknown_bytes_range_shared
    {
        unknown_bytes_range* bytes_;
        size_t* uses_;

      public:
        unknown_bytes_range_shared();
        ~unknown_bytes_range_shared();

        unknown_bytes_range_shared(const unknown_bytes_range_shared& other);
        unknown_bytes_range_shared& operator=(const unknown_bytes_range_shared& other);

        unknown_bytes_range* operator->() const;
        unknown_bytes_range& operator*() const;
    };

    class pattern_scanner_unknown
    {
        memory_range mem_rng_;
        unknown_bytes_range_shared bytes_;

      public:
        pattern_scanner_unknown(const memory_range mem_rng, const fd::string_view sig);

        void* operator()() const;
        void update(void* last_pos);
    };

    class pattern_scanner_known
    {
        memory_range mem_rng_, search_rng_;

      public:
        pattern_scanner_known(const memory_range mem_rng, const uint8_t* begin, const size_t mem_size);

        void* operator()() const;
        void update(void* last_pos);
    };

    class xrefs_finder_impl
    {
        memory_range mem_rng_;
        const uint8_t* xref_;

      public:
        xrefs_finder_impl(const memory_range mem_rng, const uintptr_t& addr);
        xrefs_finder_impl(const memory_range mem_rng, const void* addr);

        void* operator()() const;
        void update(void* last_pos);
    };

    class _Debug_creator
    {
        const void* ptr_;

      public:
        _Debug_creator(const void* ptr)
            : ptr_(ptr)
        {
        }

        void validate(const void* other) const;
        void validate(const _Debug_creator other) const;
    };

    template <typename M>
    class memory_iterator_end;

    template <typename M>
    struct memory_iterator
    {
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = size_t;
        using value_type        = void*;

        friend class memory_iterator_end<M>;

      private:
        M mem_rng_;
        void* current_;
        _Debug_creator creator_;

        void _Get_next()
        {
            mem_rng_.update(current_);
            current_ = mem_rng_();
        }

      public:
        memory_iterator(const void* creator, M mem_rng, void* current = nullptr)
            : mem_rng_(mem_rng)
            , current_(current ? current : mem_rng())
            , creator_(creator)

        {
        }

        /* template <typename Proj2>
        memory_iterator<M, std::remove_cvref_t<Proj2>> as(Proj2&& proj) const
        {
            return {
    #ifdef _DEBUG
                creator_,
    #else
                this,
    #endif
                mem_rng_,
                current_,
                std::forward<Proj2>(proj)
            };
        } */

        memory_iterator& operator++()
        {
            _Get_next();
            return *this;
        }

        memory_iterator operator++(int)
        {
            auto tmp = *this;
            _Get_next();
            return tmp;
        }

        value_type operator*() const
        {
            return current_;
        }

        bool operator==(const memory_iterator& other) const
        {
            creator_.validate(other.creator_);
            return current_ == other.current_;
        }
    };

    template <typename M>
    class memory_iterator_end
    {
        _Debug_creator creator_;

        using _Itr = memory_iterator<M>;

      public:
        memory_iterator_end([[maybe_unused]] const _Itr* itr)
            : creator_(itr->creator_)
        {
        }

        bool operator==(const _Itr& itr) const
        {
            creator_.validate(itr.creator_);
            return !itr.current_;
        }
    };

    template <typename M>
    class memory_iterator_proxy : memory_iterator<M>
    {
        using _Base = memory_iterator<M>;

        using _Begin = _Base;
        using _End   = memory_iterator_end<M>;

      public:
        using _Base::_Base;

        using _Base::difference_type;
        using _Base::iterator_category;
        using _Base::value_type;

        // using _Base::operator*;

        _Begin begin() const
        {
            return *this;
        }

        _End end() const
        {
            return this;
        }

        value_type front() const
        {
            return _Base::operator*();
        }
    };

    //--------------

    struct raw_pattern_t
    {
    };

    struct pattern_scanner : private memory_range
    {
        using memory_range::memory_range;

        using unknown_iterator = memory_iterator_proxy<pattern_scanner_unknown>;
        using known_iterator   = memory_iterator_proxy<pattern_scanner_known>;

        unknown_iterator operator()(const fd::string_view sig) const;
        known_iterator operator()(const fd::string_view sig, const raw_pattern_t) const;
        known_iterator operator()(const uint8_t* begin, const size_t mem_size) const;
    };

    struct xrefs_scanner : private memory_range
    {
        using memory_range::memory_range;

        using iterator = memory_iterator_proxy<xrefs_finder_impl>;

        template <typename T>
        iterator operator()(const T& addr) const
        {
            return {
                this, {*this, addr}
            };
        }
    };

    constexpr raw_pattern_t raw_pattern;
} // namespace fd
