module;
#include <array>
#include <cassert>
#include <functional>
#include <variant>

export module fd.callback.impl;
export import fd.callback;

template <typename T, size_t BuffSize>
class fake_vector
{
    union
    {
        uint8_t dummy_;
        std::array<T, BuffSize> view_;
    };

    size_t size_;

  public:
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

    size_t free_space() const
    {
        return BuffSize - size_;
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
        return view_.data();
    }

    T* end()
    {
        return view_.data() + size_;
    }

    const T* end() const
    {
        return view_.data() + size_;
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

template <typename T, size_t BuffSize = 1>
class buffered_vector
{
    using value_type = T;

    using fake_vec = fake_vector<value_type, BuffSize>;
    using real_vec = std::vector<value_type>;

    std::variant<fake_vec, real_vec> data_;

  public:
    buffered_vector()
        : data_(std::in_place_type<fake_vec>)
    {
        static_assert(BuffSize > 0);
    }

    void push_back(value_type&& other)
    {
        if (std::holds_alternative<real_vec>(data_))
        {
            std::get<real_vec>(data_).push_back(std::move(other));
            return;
        }
        auto& fake = std::get<fake_vec>(data_);
        if (fake.free_space() > 0)
        {
            fake.push_back(std::move(other));
        }
        else
        {
            real_vec real;
            real.reserve(fake.size() + 1);
            real.assign(std::move_iterator(fake.begin()), std::move_iterator(fake.end()));
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

template <size_t BuffSize, typename... Args>
class callback_ex : public fd::abstract_callback<Args...>
{
    using _Base = fd::abstract_callback<Args...>;

  public:
    using typename _Base::callback_type;
    using storage_type = buffered_vector<callback_type, BuffSize>;

  private:
    storage_type data_;

  public:
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
using callback = callback_ex<1, Args...>;

export namespace fd
{
    using ::callback;
    using ::callback_ex;
} // namespace fd
