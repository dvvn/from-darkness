#pragma once
#include <concepts>

namespace fd
{
struct memory_range_unpacked;

struct _memory_range
{
    using value_type = uint8_t;
    using pointer    = const uint8_t *;
    using reference  = const uint8_t &;

    using abstract_pointer = void const *;

    using iterator = pointer;

  private:
    pointer begin_;
    size_t length_;

  public:
    template <std::integral N>
    _memory_range(N begin, size_t length) requires(sizeof(N) == sizeof(abstract_pointer))
        : begin_(reinterpret_cast<pointer>(begin))
        , length_(length)
    {
    }

    _memory_range(abstract_pointer begin, size_t length)
        : begin_(static_cast<pointer>(begin))
        , length_(length)
    {
    }

    _memory_range(abstract_pointer begin, abstract_pointer end)
        : begin_(static_cast<pointer>(begin))
        , length_(static_cast<pointer>(end) - static_cast<pointer>(begin))
    {
    }

    iterator begin() const
    {
        return begin_;
    }

    iterator end() const
    {
        return begin_ + length_;
    }

    pointer data() const
    {
        return begin_;
    }

    size_t size() const
    {
        return length_;
    }

    memory_range_unpacked unpack() const;

    [[nodiscard]]
    bool update(pointer curr, size_t offset);
    [[nodiscard]]
    bool update(abstract_pointer curr, size_t offset);
};

struct memory_range_unpacked
{
    _memory_range::iterator first, end;
    size_t size;
};

struct _unknown_bytes_range;

class _pattern_updater_unknown : _memory_range
{
    using patter_pointer = void *;

    _unknown_bytes_range *bytes_;

  public:
    using value_type = patter_pointer;

    ~_pattern_updater_unknown();

    _pattern_updater_unknown(_pattern_updater_unknown const &other)            = delete;
    _pattern_updater_unknown &operator=(_pattern_updater_unknown const &other) = delete;

    _pattern_updater_unknown(_pattern_updater_unknown &&other) noexcept;
    _pattern_updater_unknown &operator=(_pattern_updater_unknown &&other) noexcept;

    _pattern_updater_unknown(_memory_range mem, _memory_range pattern);

    patter_pointer operator()() const;
    bool update(patter_pointer last_pos);
};

class _pattern_updater_known : _memory_range
{
    using patter_pointer = void *;

    _memory_range search_rng_;

  public:
    using value_type = patter_pointer;

    _pattern_updater_known(_memory_range mem_rng, abstract_pointer begin, size_t mem_size);

    patter_pointer operator()() const;
    bool update(patter_pointer last_pos);
};

class _xrefs_finder : _memory_range
{
    using xref_reference = uintptr_t const &;
    using xref_value     = uintptr_t;
    using xref_pointer   = uintptr_t const *;

    pointer xref_;

  public:
    using value_type = xref_value;

    _xrefs_finder(_memory_range mem_rng, xref_reference addr);

    xref_value operator()() const;
    bool update(xref_reference last_pos);
};

struct _memory_iterator_dbg_creator
{
#ifdef _DEBUG
  private:
    void *ptr_;

  public:
    _memory_iterator_dbg_creator(void *ptr)
        : ptr_(ptr)
    {
    }
#else
    _memory_iterator_dbg_creator(void const *ptr)
    {
    }
#endif

    void validate(void *other) const;
    void validate(_memory_iterator_dbg_creator other) const;
};

template <class Updater>
class _memory_iterator_end;

template <class Updater>
struct _memory_iterator
{
    using value_type = typename Updater::value_type;

    friend class _memory_iterator_end<Updater>;

  private:
    Updater updater_;
    value_type current_;

    [[no_unique_address]] _memory_iterator_dbg_creator creator_;

    void get_next()
    {
        current_ = updater_.update(current_) ? updater_() : value_type();
    }

  public:
    _memory_iterator(void *creator, Updater &&mem_rng, value_type current = {})
        : updater_(static_cast<Updater &&>(mem_rng))
        , current_(current)
        , creator_(creator)

    {
        if (!current)
            current_ = updater_();
    }

    _memory_iterator &operator++()
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

    bool operator==(_memory_iterator const &other) const
    {
        creator_.validate(other.creator_);
        return current_ == other.current_;
    }
};

template <class Updater>
class _memory_iterator_end
{
    [[no_unique_address]] _memory_iterator_dbg_creator creator_;

    using iter_type = _memory_iterator<Updater>;

  public:
    _memory_iterator_end(void *creator)
        : creator_(creator)
    {
    }

    bool operator==(iter_type const &itr) const
    {
        creator_.validate(itr.creator_);
        return !itr.current_;
    }
};

template <class Updater>
struct basic_memory_scanner
{
    using iterator      = _memory_iterator<Updater>;
    using last_iterator = _memory_iterator_end<Updater>;

    using value_type = typename iterator::value_type;

  private:
    iterator begin_;
    [[no_unique_address]] last_iterator end_;

  public:
    basic_memory_scanner(Updater &&updater)
        : begin_(this, static_cast<Updater &&>(updater))
        , end_(this)
    {
    }

    iterator begin() const
    {
        return begin_;
    }

    last_iterator end() const
    {
        return end_;
    }

    typename iterator::value_type front() const
    {
        return *begin_;
    }
};

//--------------

struct memory_scanner : private _memory_range
{
    using _memory_range::_memory_range;

    using finder = basic_memory_scanner<_pattern_updater_known>;

    finder operator()(abstract_pointer begin, size_t mem_size) const;
};

class pattern_scanner : _memory_range
{
    using pattern_pointer = char const *;

  public:
    using _memory_range::_memory_range;

    using scanner = basic_memory_scanner<_pattern_updater_unknown>;

    scanner operator()(pattern_pointer pattern, size_t length) const;

    [[deprecated]]
    memory_scanner raw() const;
};

struct xrefs_scanner : private _memory_range
{
    using _memory_range::_memory_range;

    using scanner = basic_memory_scanner<_xrefs_finder>;

    template <typename T>
    scanner operator()(T const &addr) const
    {
        return {
            {*this, addr}
        };
    }
};
} // namespace fd