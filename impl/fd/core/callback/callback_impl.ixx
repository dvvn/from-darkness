module;
#include <algorithm>
#include <array>
#include <cassert>
#include <functional>
#include <limits>
#include <variant>

export module fd.callback.impl;
export import fd.callback;

template <size_t S>
constexpr auto _Smallest_size_type()
{
    if constexpr (S <= std::numeric_limits<uint8_t>::max())
        return uint8_t();
    else if constexpr (S <= std::numeric_limits<uint16_t>::max())
        return uint16_t();
    else if constexpr (S <= std::numeric_limits<uint32_t>::max())
        return uint32_t();
    else
        return uint64_t();
}

template <typename T, size_t ExtraBuffSize>
class fake_vector
{
    using size_type = decltype(_Smallest_size_type<sizeof(std::vector<T>) + sizeof(ExtraBuffSize) * sizeof(T)>());

    union
    {
        uint8_t dummy_;
        std::array<T, std::max<size_t>(1, (sizeof(std::vector<T>) - sizeof(size_type)) / sizeof(T) + ExtraBuffSize)> view_;
    };

    size_type size_;

  public:
    void mark_empty()
    {
        size_ = 0;
    }

    ~fake_vector()
    {
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
            while (size_ > 0)
                std::destroy_at(&view_[--size_]);
        }
        else
        {
            size_ = 0;
        }
    }

    fake_vector()
        : size_(0)
    {
    }

    fake_vector(const fake_vector& other)
    {
        *this = other;
    }

    fake_vector(fake_vector&& other)
    {
        *this = std::move(other);
    }

    fake_vector& operator=(const fake_vector& other)
    {
        static_assert(std::is_copy_assignable_v<T>);
        if constexpr (std::is_trivially_copy_assignable_v<T> && std::is_trivially_destructible_v<T>)
        {
            std::copy(other.begin(), other.end(), begin());
            size_ = other.size_;
        }
        else
        {
            std::destroy_at(this);
            for (size_t i = 0; i < other.size_; ++i)
                push_back(other[i]);
        }
        return *this;
    }

    fake_vector& operator=(fake_vector&& other)
    {
        static_assert(std::is_move_assignable_v<T>);
        if constexpr (!std::is_class_v<T>)
        {
            *this = other;
        }
        else
        {
            std::destroy_at(this);
            for (size_t i = 0; i < other.size_; ++i)
                push_back(std::move(other[i]));
        }

        return *this;
    }

    //------------

    void push_back(const T& obj)
    {
        auto& pos = view_[size_++];
        if constexpr (std::is_class_v<T>)
            std::construct_at(&pos, obj);
        else
            pos = obj;
    }

    void push_back(T&& obj)
    {
        if constexpr (std::is_class_v<T>)
            std::construct_at(&view_[size_++], std::move(obj));
        else
            push_back(obj);
    }

    void pop_back()
    {
        if constexpr (std::is_class_v<T>)
            std::destroy_at(&view_[size_--]);
        else
            --size_;
    }

    //-----------------

    static constexpr size_t capacity()
    {
        return sizeof(view_) / sizeof(T);
    }

    size_t size() const
    {
        return size_;
    }

    T* begin()
    {
        return view_.data();
    }

    const T* begin() const
    {
        return const_cast<fake_vector*>(this)->begin();
    }

    T* end()
    {
        return view_.data() + size_;
    }

    const T* end() const
    {
        return const_cast<fake_vector*>(this)->end();
    }

    T& operator[](const size_t idx)
    {
        return view_[idx];
    }

    const T& operator[](const size_t idx) const
    {
        return view_[idx];
    }
};

template <typename T, size_t ExtraBuffSize = 0>
class buffered_vector
{
    using value_type = T;

    using real_vec = std::vector<value_type>;
    using fake_vec = fake_vector<value_type, ExtraBuffSize>;

    std::variant<real_vec, fake_vec> data_;

  public:
    static constexpr size_t known_buffer_size()
    {
        return fake_vec::capacity();
    }

    buffered_vector()
        : data_(std::in_place_type<fake_vec>)
    {
    }

    void push_back(value_type&& other)
    {
        if (std::holds_alternative<real_vec>(data_))
        {
            std::get<real_vec>(data_).push_back(std::move(other));
            return;
        }
        auto& fake = std::get<fake_vec>(data_);
        if (fake.capacity() - fake.size() > 0)
        {
            fake.push_back(std::move(other));
        }
        else
        {
            real_vec real;
            real.reserve(fake.size() + 1);
            real.assign(std::move_iterator(fake.begin()), std::move_iterator(fake.end()));
            fake.mark_empty();
            real.push_back(std::move(other));
            data_.emplace<real_vec>(std::move(real));
        }
    }

    bool empty() const
    {
        return std::visit(
            [](auto& vec) {
                return vec.size() == 0;
            },
            data_);
    }

    value_type* begin()
    {
        return std::visit(
            [](auto& vec) {
                return &vec[0];
            },
            data_);
    }

    value_type* end()
    {
        return std::visit(
            [](auto& vec) {
                return &vec[0] + vec.size();
            },
            data_);
    }

    const value_type* begin() const
    {
        return const_cast<buffered_vector*>(this)->begin();
    }

    const value_type* end() const
    {
        return const_cast<buffered_vector*>(this)->end();
    }
};

template <size_t ExtraBuffSize, typename... Args>
class callback_ex : public fd::abstract_callback<Args...>
{
    using _Base = fd::abstract_callback<Args...>;

  public:
    using typename _Base::callback_type;
    using storage_type = buffered_vector<callback_type, ExtraBuffSize>;

  private:
    storage_type data_;

  public:
    static constexpr size_t known_buffer_size()
    {
        return storage_type::known_buffer_size();
    }

    void append(callback_type&& callback) override
    {
        data_.push_back(std::move(callback));
    }

    void invoke(Args... args) const override
    {
        for (auto& fn : data_)
            std::invoke(fn, args...);
    }

    bool empty() const override
    {
        return data_.empty();
    }
};

template <typename... Args>
using callback = callback_ex<0, Args...>;

export namespace fd
{
    using ::callback;
    using ::callback_ex;
} // namespace fd
