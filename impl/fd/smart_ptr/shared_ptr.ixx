module;

#include <memory>
#include <typeinfo>
#include <utility>

export module fd.smart_ptr.shared;

namespace fd
{
    export template <typename T, typename S = size_t>
    struct weak_ptr;

    export template <typename T, typename S = size_t>
    struct shared_ptr;

    template <class T>
    void destroy_class(T& obj)
    {
        if constexpr (std::is_class_v<T> || std::is_union_v<T>)
            std::destroy_at(&obj);
    }

    using type_info_ref = const std::type_info&;

    template <typename T>
    struct basic_controller
    {
        virtual ~basic_controller() = default;

        virtual T* get() const             = 0;
        virtual type_info_ref type() const = 0;

        virtual void destroy() = 0;
    };

    constexpr auto aa = std::is_trivially_destructible_v<int>;

    template <typename S>
    class ref_counter final
    {
        union
        {
            uint8_t gap_;
            S counter_;
        };

        [[no_unique_address]] std::conditional_t<std::is_trivially_destructible_v<S>, std::false_type, bool> destroyed_;

      public:
        using size_type = decltype([] {
            S dummy_;
            return dummy_++;
        }());

        ~ref_counter()
        {
            if constexpr (!std::is_trivially_destructible_v<S>)
            {
                if (!destroyed_)
                {
                    destroy_class(counter_);
                    destroyed_ = true;
                }
            }
        }

        ref_counter()
            : counter_(0u)
        {
        }

        ref_counter(const ref_counter&)            = delete;
        ref_counter& operator=(const ref_counter&) = delete;

        size_type operator++()
        {
            return ++counter_;
        }

        size_type operator--()
        {
            return --counter_;
        }

        size_type load() const
        {
            return counter_;
        }

        void _Destroy()
        {
            destroy_class(counter_);
        }
    };

    template <typename T, typename S>
    struct controller : basic_controller<T>
    {
        ref_counter<S> strong, weak;

        void _Destroy_strong()
        {
            if (--strong > 0)
                return;
            this->destroy();
            if (weak.load() == 0)
                delete this;
        }

        void _Destroy_weak()
        {
            if (--weak > 0)
                return;
            if (strong.load() == 0)
                delete this;
        }
    };

    enum class data_type : uint8_t
    {
        strong,
        weak
    };

    template <data_type Type>
    struct updater;

    template <>
    struct updater<data_type::strong>
    {
        template <class D>
        static void construct(D*& data, D* new_data)
        {
#ifdef _DEBUG
            if (!new_data)
                throw std::bad_alloc();
#endif
            ++new_data->strong;
            data = new_data;
        }

        template <class D>
        static void destroy(D* data)
        {
            if (data)
                data->_Destroy_strong();
        }
    };

    template <>
    struct updater<data_type::weak>
    {
        template <class D>
        static void construct(D*& data, D* new_data)
        {
#ifdef _DEBUG
            if (!new_data || new_data->strong.load() == 0)
                throw std::bad_weak_ptr();
#endif
            ++new_data->weak;
            data = new_data;
        }

        template <class D>
        static void destroy(D* data)
        {
            if (data)
                data->_Destroy_weak();
        }
    };

    struct unpacker
    {
        template <class H>
        static auto get(const H& owner)
        {
            return owner.ctrl_;
        }
    };

    template <typename T, typename S, data_type Type>
    struct data_owner
    {
        using element_type = std::remove_extent_t<T>;
        using pointer      = element_type*;
        using reference    = element_type&;

      private:
        using _Controller = controller<element_type, S>;

        friend struct unpacker;

        _Controller* ctrl_;

      public:
        data_owner()
            : ctrl_(nullptr)
        {
        }

        void _Construct(_Controller* new_data)
        {
            updater<Type>::construct(ctrl_, new_data);
        }

        template <data_type Type2>
        void _Construct(const data_owner<T, S, Type2>& holder)
        {
            _Construct(unpacker::get(holder));
        }

        void _Destroy()
        {
            updater<Type>::destroy(ctrl_);
        }

        void _Swap(data_owner& from)
        {
            using std::swap;
            swap(ctrl_, from.ctrl_);
        }

        // swap

        void reset()
        {
            this->_Destroy();
            ctrl_ = nullptr;
        }

        // get_deleter

        explicit operator bool() const
        {
            return ctrl_;
        }

        typename ref_counter<S>::size_type use_count() const
        {
            return ctrl_ ? ctrl_->strong.load() : 0;
        }

        type_info_ref type() const
        {
            if constexpr (Type == data_type::weak)
            {
                if (!ctrl_)
                    throw std::bad_typeid();
            }

            return ctrl_->type();
        }
    };

    template <typename T, typename S>
    struct weak_ptr : data_owner<T, S, data_type::weak>
    {
        using element_type = std::remove_extent_t<T>;
        using shared_type  = shared_ptr<T, S>;

        ~weak_ptr()
        {
            this->_Destroy();
        }

        weak_ptr() = default;

        weak_ptr(const shared_type& other);
        weak_ptr(shared_type&& other) = delete;

        weak_ptr(const weak_ptr& other)
        {
            this->_Construct(other);
        }

        weak_ptr(weak_ptr&& other)
        {
            this->_Swap(other);
        }

        weak_ptr& operator=(const shared_type& other)
        {
            this->_Destroy();
            this->_Construct(other);
            return *this;
        }

        weak_ptr& operator=(shared_type&& other) = delete;

        weak_ptr& operator=(const weak_ptr& other)
        {
            this->_Destroy();
            this->_Construct(other);
            return *this;
        }

        weak_ptr& operator=(weak_ptr&& other)
        {
            this->_Swap(other);
            return *this;
        }

        bool expired() const
        {
            return this->use_count() == 0;
        }

        shared_type lock() const;
    };

    template <typename T, typename S>
    struct shared_ptr : data_owner<T, S, data_type::strong>
    {
        using element_type = std::remove_extent_t<T>;
        using pointer      = element_type*;
        using reference    = element_type&;

        using weak_type = weak_ptr<T, S>;

      private:
        using _Controller = controller<T, S>;

      public:
        ~shared_ptr()
        {
            this->_Destroy();
        }

        shared_ptr() = default;

        shared_ptr(shared_ptr&& other)
        {
            this->_Swap(other);
        }

        shared_ptr(const shared_ptr& other)
        {
            this->_Construct(other);
        }

        template <typename T1, typename D = default_delete<T1>>
        shared_ptr(T1* ptr, D deleter = {})
        {
            class _External_data : public _Controller
            {
                // unique_ptr<T1, D> ptr_;
                T1* ptr_;
                [[no_unique_address]] D deleter_;

              public:
                _External_data(T1* ptr, D& deleter)
                    : ptr_(ptr)
                    , deleter_(std::move(deleter))
                {
                }

                pointer get() const override
                {
                    return ptr_;
                }

                type_info_ref type() const override
                {
                    return typeid(T);
                }

                void destroy() override
                {
                    // std::destroy_at(&data_.ptr);
                    if (ptr_)
                        deleter_(ptr_);
                    destroy_class(deleter_);
                }
            };

            this->_Construct(new _External_data(ptr, deleter));
        }

        template <typename T1>
        shared_ptr(T1&& value)
        {
            class _Innter_data : public _Controller
            {
                using value_type = std::remove_cvref_t<T1>;
                value_type value_;

              public:
                _Innter_data(T1&& value)
                    : value_(std::forward<T1>(value))
                {
                    static_assert(std::convertible_to<std::add_pointer_t<value_type>, pointer>);
                }

                pointer get() const override
                {
                    return const_cast<value_type*>(&value_);
                }

                type_info_ref type() const override
                {
                    return typeid(value_type);
                }

                void destroy() override
                {
                    destroy_class(value_);
                }
            };

            this->_Construct(new _Innter_data(std::forward<T1>(value)));
        }

        shared_ptr& operator=(const shared_ptr& other)
        {
            this->_Destroy();
            this->_Construct(other);
            return *this;
        }

        shared_ptr& operator=(shared_ptr&& other)
        {
            this->_Swap(other);
            return *this;
        }

        pointer get() const
        {
            return unpacker::get(*this)->get();
        }

        reference operator*() const requires(!std::is_unbounded_array_v<T>)
        {
            return *this->get();
        }

        pointer operator->() const requires(!std::is_unbounded_array_v<T>)
        {
            return this->get();
        }

        reference operator[](const size_t idx) const requires(std::is_unbounded_array_v<T>) 
        {
            return this->get()[idx];
        }
    };

    template <typename T, typename S>
    weak_ptr<T, S>::weak_ptr(const shared_type& other)
    {
        this->_Construct(other);
    }

    template <typename T, typename S>
    auto weak_ptr<T, S>::lock() const -> shared_type
    {
        shared_type tmp;
        tmp._Construct(*this);
        return tmp;
    }

    template <typename T>
    shared_ptr(T*) -> shared_ptr<T>;
    template <typename T>
    shared_ptr(T&&) -> shared_ptr<std::remove_cvref_t<T>>;
    template <typename... Ts>
    shared_ptr(const shared_ptr<Ts...>&) -> shared_ptr<Ts...>;

    template <typename T, typename S>
    weak_ptr(const shared_ptr<T, S>&) -> weak_ptr<T, S>;
    template <typename T, typename S>
    shared_ptr(const weak_ptr<T, S>&) -> shared_ptr<T, S>;

} // namespace fd
