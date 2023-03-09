#pragma once

#include <span>

namespace fd
{
struct _memory_range : std::span<uint8_t const>
{
    template <typename It>
    _memory_range(It begin, It end)
        : std::span<uint8_t const>(begin, end)
    {
    }

    template <typename It>
    _memory_range(It begin, size_t size)
        : std::span<uint8_t const>(static_cast<pointer>(begin), size)
    {
    }

    _memory_range(std::span<uint8_t> rng)
        : std::span<uint8_t const>(rng)
    {
    }

    [[nodiscard]]
    bool update(uint8_t const *curr, size_t offset);
    [[nodiscard]]
    bool update(void const *curr, size_t offset);
};

class _unknown_bytes_range;

class _pattern_updater_unknown
{
    _memory_range mem_rng_;
    _unknown_bytes_range *bytes_;

  public:
    ~_pattern_updater_unknown();

    _pattern_updater_unknown(_pattern_updater_unknown const &other)            = delete;
    _pattern_updater_unknown &operator=(_pattern_updater_unknown const &other) = delete;

    _pattern_updater_unknown(_pattern_updater_unknown &&other) noexcept;
    _pattern_updater_unknown &operator=(_pattern_updater_unknown &&other) noexcept;

    _pattern_updater_unknown(_memory_range mem_rng, std::span<char const> sig);
    _pattern_updater_unknown(_memory_range mem_rng, uint8_t const *begin, size_t mem_size);

    void *operator()() const;
    bool update(void const *last_pos);
};

class _pattern_updater_known
{
    _memory_range mem_rng_, search_rng_;

  public:
    _pattern_updater_known(_memory_range mem_rng, uint8_t const *begin, size_t mem_size);

    void *operator()() const;
    bool update(void const *last_pos);
};

class _xrefs_finder
{
    _memory_range mem_rng_;
    uint8_t const *xref_;

  public:
    _xrefs_finder(_memory_range mem_rng, uintptr_t const &addr);
    _xrefs_finder(_memory_range mem_rng, void const *&addr);

    void *operator()() const;
    bool update(void const *last_pos);
};

struct _memory_iterator_dbg_creator
{
#ifdef _DEBUG
  private:
    void const *ptr_;

  public:
    _memory_iterator_dbg_creator(void const *ptr)
        : ptr_(ptr)
    {
    }
#else
    _memory_iterator_dbg_creator(void const *ptr)
    {
    }
#endif

    void validate(void const *other) const;
    void validate(_memory_iterator_dbg_creator other) const;
};

template <typename M>
class _memory_iterator_end;

template <typename M>
struct _memory_iterator
{
    using value_type = void *;

    friend class _memory_iterator_end<M>;

  private:
    M mem_rng_;
    void *current_;

    [[no_unique_address]] _memory_iterator_dbg_creator creator_;

    void get_next()
    {
        current_ = mem_rng_.update(current_) ? mem_rng_() : nullptr;
    }

  public:
    _memory_iterator(void const *creator, M &&mem_rng, void *current = nullptr)
        : mem_rng_(std::move(mem_rng))
        , current_(current ? current : mem_rng_())
        , creator_(creator)

    {
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

template <typename M>
class _memory_iterator_end
{
    [[no_unique_address]] _memory_iterator_dbg_creator creator_;

    using iter_type = _memory_iterator<M>;

  public:
    _memory_iterator_end(void const *creator)
        : creator_(creator)
    {
    }

    bool operator==(iter_type const &itr) const
    {
        creator_.validate(itr.creator_);
        return !itr.current_;
    }
};

template <typename M>
class _memory_finder
{
    _memory_iterator<M> begin_;
    [[no_unique_address]] _memory_iterator_end<M> end_;

  public:
    _memory_finder(M &&mem_rng)
        : begin_(this, std::move(mem_rng))
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

    finder operator()(std::span<char const> sig) const;
    finder operator()(void const *begin, size_t mem_size) const;
};

struct pattern_scanner_text : private _memory_range
{
    using _memory_range::_memory_range;

    using finder = _memory_finder<_pattern_updater_unknown>;

    finder operator()(std::span<char const> sig) const;
    finder operator()(void const *begin, size_t mem_size) const;

    pattern_scanner_raw raw() const;
};

struct pattern_scanner : private _memory_range
{
    using _memory_range::_memory_range;

    using unknown_finder = _memory_finder<_pattern_updater_unknown>;
    using known_finder   = _memory_finder<_pattern_updater_known>;

    unknown_finder operator()(std::span<char const> sig) const;
    known_finder operator()(uint8_t const *begin, size_t mem_size) const;

    pattern_scanner_raw raw() const;
};

struct xrefs_scanner : private _memory_range
{
    using _memory_range::_memory_range;

    using finder = _memory_finder<_xrefs_finder>;

    template <typename T>
    finder operator()(T const &addr) const
    {
        return {
            {*this, addr}
        };
    }
};
} // namespace fd