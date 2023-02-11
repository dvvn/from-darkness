#pragma once

#include <fd/views.h>

namespace fd
{
struct memory_range : range_view<const uint8_t*>
{
#ifdef _DEBUG
    memory_range(const uint8_t* from, const uint8_t* to);
    memory_range(const uint8_t* from, size_t size);
    memory_range(range_view rng);
#else
    using range_view<const uint8_t*>::range_view;
#endif

    void update(const uint8_t* curr, size_t offset = 0);
    void update(const void* curr, size_t offset = 0);
};

class unknown_bytes_range;

class unknown_bytes_range_shared
{
    unknown_bytes_range* bytes_;
    size_t*              uses_;

  public:
    unknown_bytes_range_shared();
    ~unknown_bytes_range_shared();

    unknown_bytes_range_shared(const unknown_bytes_range_shared& other);
    unknown_bytes_range_shared& operator=(const unknown_bytes_range_shared& other);

    unknown_bytes_range* operator->() const;
    unknown_bytes_range& operator*() const;
};

class pattern_updater_unknown
{
    memory_range               memRng_;
    unknown_bytes_range_shared bytes_;

  public:
    pattern_updater_unknown(memory_range memRng, range_view<const char*> sig);
    pattern_updater_unknown(memory_range memRng, const uint8_t* begin, size_t memSize);

    void* operator()() const;
    void  update(const void* lastPos);
};

class pattern_updater_known
{
    memory_range memRng_, searchRng_;

  public:
    pattern_updater_known(memory_range memRng, const uint8_t* begin, size_t memSize);

    void* operator()() const;
    void  update(const void* lastPos);
};

class xrefs_finder_impl
{
    memory_range   memRng_;
    const uint8_t* xref_;

  public:
    xrefs_finder_impl(memory_range memRng, const uintptr_t& addr);
    xrefs_finder_impl(memory_range memRng, const void*& addr);

    void* operator()() const;
    void  update(const void* lastPos);
};

struct memory_iterator_dbg_creator
{
#ifdef _DEBUG
  private:
    const void* ptr_;

  public:
    memory_iterator_dbg_creator(const void* ptr)
        : ptr_(ptr)
    {
    }
#else
    memory_iterator_dbg_creator(const void* ptr)
    {
    }
#endif

    void validate(const void* other) const;
    void validate(memory_iterator_dbg_creator other) const;
};

template <typename M>
class memory_iterator_end;

template <typename M>
struct memory_iterator
{
    using value_type = void*;

    friend class memory_iterator_end<M>;

  private:
    M     memRng_;
    void* current_;

    [[no_unique_address]] memory_iterator_dbg_creator creator_;

    void get_next()
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

    memory_iterator& operator++()
    {
        get_next();
        return *this;
    }

    memory_iterator operator++(int)
    {
        auto tmp = *this;
        get_next();
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
    [[no_unique_address]] memory_iterator_dbg_creator creator_;

    using iter_type = memory_iterator<M>;

  public:
    memory_iterator_end(const void* creator)
        : creator_(creator)
    {
    }

    bool operator==(const iter_type& itr) const
    {
        creator_.validate(itr.creator_);
        return !itr.current_;
    }
};

template <typename M>
class memory_finder
{
    memory_iterator<M>                           begin_;
    [[no_unique_address]] memory_iterator_end<M> end_;

  public:
    memory_finder(M memRng)
        : begin_(this, memRng)
        , end_(this)
    {
    }

    auto begin() const
    {
        return begin_;
    }

    auto end() const
    {
        return end_;
    }

    auto front() const
    {
        return *begin_;
    }
};

//--------------

struct pattern_scanner_raw : private memory_range
{
    using memory_range::memory_range;

    using finder = memory_finder<pattern_updater_known>;

    finder operator()(range_view<const char*> sig) const;
    finder operator()(const uint8_t* begin, size_t memSize) const;
};

struct pattern_scanner_text : private memory_range
{
    using memory_range::memory_range;

    using finder = memory_finder<pattern_updater_unknown>;

    finder operator()(range_view<const char*> sig) const;
    finder operator()(const uint8_t* begin, size_t memSize) const;

    pattern_scanner_raw raw() const;
};

struct pattern_scanner : private memory_range
{
    using memory_range::memory_range;

    using unknown_finder = memory_finder<pattern_updater_unknown>;
    using known_finder   = memory_finder<pattern_updater_known>;

    unknown_finder operator()(range_view<const char*> sig) const;
    known_finder   operator()(const uint8_t* begin, size_t memSize) const;

    pattern_scanner_raw raw() const;
};

struct xrefs_scanner : private memory_range
{
    using memory_range::memory_range;

    using finder = memory_finder<xrefs_finder_impl>;

    template <typename T>
    finder operator()(const T& addr) const
    {
        return {
            {*this, addr}
        };
    }
};
} // namespace fd