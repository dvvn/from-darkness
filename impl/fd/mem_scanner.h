#pragma once

#include <fd/views.h>

namespace fd
{
struct _memory_range : range_view<const uint8_t*>
{
#ifdef _DEBUG
    _memory_range(const uint8_t* from, const uint8_t* to);
    _memory_range(const uint8_t* from, size_t size);
    _memory_range(range_view rng);
#else
    using range_view<const uint8_t*>::range_view;
#endif

    void update(const uint8_t* curr, size_t offset = 0);
    void update(const void* curr, size_t offset = 0);
};

class _unknown_bytes_range;

class _unknown_bytes_range_shared
{
    _unknown_bytes_range* bytes_;
    size_t*               uses_;

  public:
    _unknown_bytes_range_shared();
    ~_unknown_bytes_range_shared();

    _unknown_bytes_range_shared(const _unknown_bytes_range_shared& other);
    _unknown_bytes_range_shared& operator=(const _unknown_bytes_range_shared& other);

    _unknown_bytes_range* operator->() const;
    _unknown_bytes_range& operator*() const;
};

class _pattern_updater_unknown
{
    _memory_range               memRng_;
    _unknown_bytes_range_shared bytes_;

  public:
    _pattern_updater_unknown(_memory_range memRng, range_view<const uint16_t*> bytes);
    _pattern_updater_unknown(_memory_range memRng, range_view<const char*> sig);
    _pattern_updater_unknown(_memory_range memRng, const uint8_t* begin, size_t memSize);

    void* operator()() const;
    void  update(const void* lastPos);
};

class _pattern_updater_known
{
    _memory_range memRng_, searchRng_;

  public:
    _pattern_updater_known(_memory_range memRng, const uint8_t* begin, size_t memSize);

    void* operator()() const;
    void  update(const void* lastPos);
};

class _xrefs_finder
{
    _memory_range  memRng_;
    const uint8_t* xref_;

  public:
    _xrefs_finder(_memory_range memRng, const uintptr_t& addr);
    _xrefs_finder(_memory_range memRng, const void*& addr);

    void* operator()() const;
    void  update(const void* lastPos);
};

struct _memory_iterator_dbg_creator
{
#ifdef _DEBUG
  private:
    const void* ptr_;

  public:
    _memory_iterator_dbg_creator(const void* ptr)
        : ptr_(ptr)
    {
    }
#else
    _memory_iterator_dbg_creator(const void* ptr)
    {
    }
#endif

    void validate(const void* other) const;
    void validate(_memory_iterator_dbg_creator other) const;
};

template <typename M>
class _memory_iterator_end;

template <typename M>
struct _memory_iterator
{
    using value_type = void*;

    friend class _memory_iterator_end<M>;

  private:
    M     memRng_;
    void* current_;

    [[no_unique_address]] _memory_iterator_dbg_creator creator_;

    void get_next()
    {
        memRng_.update(current_);
        current_ = memRng_();
    }

  public:
    _memory_iterator(const void* creator, M memRng, void* current = nullptr)
        : memRng_(memRng)
        , current_(current ? current : memRng())
        , creator_(creator)

    {
    }

    _memory_iterator& operator++()
    {
        get_next();
        return *this;
    }

    _memory_iterator operator++(int)
    {
        auto tmp = *this;
        get_next();
        return tmp;
    }

    value_type operator*() const
    {
        return current_;
    }

    bool operator==(const _memory_iterator& other) const
    {
        creator_.validate(other.creator_);
        return current_ == other.current_;
    }
};

template <typename M>
class _memory_iterator_end
{
    [[no_unique_address]] _memory_iterator_dbg_creator creator_;

    using iter_type = _memory_iterator<M>;

  public:
    _memory_iterator_end(const void* creator)
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
class _memory_finder
{
    _memory_iterator<M>                           begin_;
    [[no_unique_address]] _memory_iterator_end<M> end_;

  public:
    _memory_finder(M memRng)
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

struct pattern_scanner_raw : private _memory_range
{
    using _memory_range::_memory_range;

    using finder = _memory_finder<_pattern_updater_known>;

    finder operator()(range_view<const char*> sig) const;
    finder operator()(const uint8_t* begin, size_t memSize) const;
};

struct pattern_scanner_text : private _memory_range
{
    using _memory_range::_memory_range;

    using finder = _memory_finder<_pattern_updater_unknown>;

    finder operator()(range_view<const char*> sig) const;
    finder operator()(const uint8_t* begin, size_t memSize) const;

    pattern_scanner_raw raw() const;
};

struct pattern_scanner : private _memory_range
{
    using _memory_range::_memory_range;

    using unknown_finder = _memory_finder<_pattern_updater_unknown>;
    using known_finder   = _memory_finder<_pattern_updater_known>;

    unknown_finder operator()(range_view<const uint16_t*> sig) const; // for unknown_bytes_range_ct
    unknown_finder operator()(range_view<const char*> sig) const;
    known_finder   operator()(const uint8_t* begin, size_t memSize) const;

    pattern_scanner_raw raw() const;
};

struct xrefs_scanner : private _memory_range
{
    using _memory_range::_memory_range;

    using finder = _memory_finder<_xrefs_finder>;

    template <typename T>
    finder operator()(const T& addr) const
    {
        return {
            {*this, addr}
        };
    }
};
} // namespace fd