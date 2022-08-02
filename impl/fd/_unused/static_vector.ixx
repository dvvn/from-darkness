module;

#include <algorithm>
#include <array>
#include <ranges>
#include <variant>
#include <vector>

export module fd.static_vector;

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

template <typename T, size_t ExtraBuffSize = 0>
class static_vector
{
    using size_type = decltype(_Smallest_size_type<sizeof(std::vector<T>) + sizeof(ExtraBuffSize) * sizeof(T)>());

    static constexpr size_t num_elements_ = std::max<size_t>(1, (sizeof(std::vector<T>) - sizeof(size_type)) / sizeof(T) + ExtraBuffSize);

    union
    {
        uint8_t dummy_;
        std::array<T, num_elements_> view_;
    };

    size_type size_;

    void pop_back_until(const size_type target_size)
    {
        while (size_ > target_size)
            pop_back();
    }

  public:
    void mark_empty()
    {
        size_ = 0;
    }

    ~static_vector()
    {
        pop_back_until(0);
    }

    static_vector()
        : size_(0)
    {
    }

    static_vector(const static_vector& other)
    {
        *this = other;
    }

    static_vector(static_vector&& other)
    {
        *this = std::move(other);
    }

    static_vector& operator=(const static_vector& other)
    {
        pop_back_until(other.size_);
        std::ranges::copy(other, begin());
        return *this;
    }

    static_vector& operator=(static_vector&& other)
    {
        pop_back_until(other.size_);
        std::ranges::move(other, begin());
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
            std::destroy_at(&view_[--size_]);
        else
            --size_;
    }

    //-----------------

    static constexpr size_t capacity()
    {
        return num_elements_;
    }

    size_type size() const
    {
        return size_;
    }

    bool empty() const
    {
        return size_ == 0;
    }

    T* data()
    {
        return view_.data();
    }

    const T* data() const
    {
        return const_cast<static_vector*>(this)->data();
    }

    T* begin()
    {
        return data();
    }

    const T* begin() const
    {
        return data();
    }

    T* end()
    {
        return view_.data() + size_;
    }

    const T* end() const
    {
        return const_cast<static_vector*>(this)->end();
    }

    T& operator[](const size_type idx)
    {
        return view_[idx];
    }

    const T& operator[](const size_type idx) const
    {
        return view_[idx];
    }
};

template <typename T, size_t ExtraBuffSize = 0>
class static_vector_resizable
{
    using value_type = T;

    using real_vec = std::vector<value_type>;
    using fake_vec = static_vector<value_type, ExtraBuffSize>;

    std::variant<real_vec, fake_vec> data_;

    template <typename Q>
    void push_back_impl(Q&& val)
    {
        if (std::holds_alternative<real_vec>(data_))
        {
            std::get<real_vec>(data_).push_back(std::forward<Q>(val));
            return;
        }
        auto& fake = std::get<fake_vec>(data_);
        if (fake.capacity() - fake.size() > 0)
        {
            fake.push_back(std::forward<Q>(val));
        }
        else
        {
            real_vec real;
            real.reserve(fake.size() + 1);
            real.assign(std::move_iterator(fake.begin()), std::move_iterator(fake.end()));
            fake.mark_empty();
            real.push_back(std::forward<Q>(val));
            data_.emplace<real_vec>(std::move(real));
        }
    }

  public:
    static_vector_resizable()
        : data_(std::in_place_type<fake_vec>)
    {
    }

    void push_back(value_type&& other)
    {
        push_back_impl(std::move(other));
    }

    void push_back(const value_type& other)
    {
        push_back_impl(other);
    }

    void pop_back()
    {
        std::visit(
            [](auto& vec) {
                vec.pop_back();
            },
            data_);
    }

    bool empty() const
    {
        return std::visit(
            [](auto& vec) {
                return vec.empty();
            },
            data_);
    }

    size_t size() const
    {
        return std::visit(
            [](auto& vec) {
                return static_cast<size_t>(vec.size());
            },
            data_);
    }

    value_type* begin()
    {
        return std::visit(
            [](auto& vec) {
                return vec.data();
            },
            data_);
    }

    value_type* end()
    {
        return std::visit(
            [](auto& vec) {
                return vec.data() + vec.size();
            },
            data_);
    }

    const value_type* begin() const
    {
        return const_cast<static_vector*>(this)->begin();
    }

    const value_type* end() const
    {
        return const_cast<static_vector*>(this)->end();
    }
};

export namespace fd
{
    using ::static_vector;
    using ::static_vector_resizable;
} // namespace fd
