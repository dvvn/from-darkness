#pragma once

#include <fd/string.h>

#include <iterator>

namespace fd
{
    struct memory_range
    {
        const uint8_t *from, *to;

        memory_range(const uint8_t* from, const uint8_t* to);
        memory_range(const uint8_t* from, size_t size);

        void update(const uint8_t* curr, size_t offset = 0);
        void update(const void* curr, size_t offset = 0);
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
        memory_range memRng_;
        unknown_bytes_range_shared bytes_;

      public:
        pattern_scanner_unknown(memory_range memRng, string_view sig);

        void* operator()() const;
        void update(const void* lastPos);
    };

    class pattern_scanner_known
    {
        memory_range memRng_, searchRng_;

      public:
        pattern_scanner_known(memory_range memRng, const uint8_t* begin, size_t memSize);

        void* operator()() const;
        void update(void* lastPos);
    };

    class xrefs_finder_impl
    {
        memory_range memRng_;
        const uint8_t* xref_;

      public:
        xrefs_finder_impl(memory_range memRng, const uintptr_t& addr);
        xrefs_finder_impl(memory_range memRng, const void* addr);

        void* operator()() const;
        void update(const void* lastPos);
    };

    class memory_iterator_dbg_creator
    {
        const void* ptr_;

      public:
        memory_iterator_dbg_creator(const void* ptr)
            : ptr_(ptr)
        {
        }

        void validate(const void* other) const;
        void validate(memory_iterator_dbg_creator other) const;
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
        M memRng_;
        void* current_;
        memory_iterator_dbg_creator creator_;

        void _Get_next()
        {
            memRng_.update(current_);
            current_ = memRng_();
        }

      public:
        memory_iterator(const void* creator, M memRng, void* current = nullptr)
            : memRng_(memRng)
            , current_(current ? current : memRng())
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
                memRng_,
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
        memory_iterator_dbg_creator creator_;

        using iter_type = memory_iterator<M>;

      public:
        memory_iterator_end(const iter_type* itr)
            : creator_(itr->creator_)
        {
        }

        bool operator==(const iter_type& itr) const
        {
            creator_.validate(itr.creator_);
            return !itr.current_;
        }
    };

    template <typename M>
    class memory_iterator_proxy : memory_iterator<M>
    {
        using base_type = memory_iterator<M>;

        using begin_type = base_type;
        using end_type   = memory_iterator_end<M>;

      public:
        using base_type::base_type;

        using base_type::difference_type;
        using base_type::iterator_category;
        using base_type::value_type;

        using base_type::operator*;

        // for algorithm's only
        begin_type begin() const
        {
            return *this;
        }

        // for algorithm's only
        end_type end() const
        {
            return this;
        }

        /*value_type front() const
        {
            return base_type::operator*();
        }*/
    };

    //--------------

    struct pattern_scanner_raw : private memory_range
    {
        using memory_range::memory_range;

        using iterator = memory_iterator_proxy<pattern_scanner_known>;

        iterator operator()(string_view sig) const;
        iterator operator()(const uint8_t* begin, size_t memSize) const;
    };

    struct pattern_scanner : private memory_range
    {
        using memory_range::memory_range;

        using unknown_iterator = memory_iterator_proxy<pattern_scanner_unknown>;
        using known_iterator   = memory_iterator_proxy<pattern_scanner_known>;

        unknown_iterator operator()(string_view sig) const;
        known_iterator operator()(const uint8_t* begin, size_t memSize) const;

        pattern_scanner_raw raw() const;
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
} // namespace fd
