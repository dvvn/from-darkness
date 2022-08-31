module;

#include <compare>

export module fd.address;

template <typename From, typename To>
concept reinterpret_convertible_to = requires(From val)
{
    reinterpret_cast<To>(val);
};

template <typename From, typename To>
concept static_convertible_to = requires(From val)
{
    static_cast<To>(val);
};

template <typename Out, typename In>
Out _Force_cast(In in)
{
    if constexpr (reinterpret_convertible_to<In, Out>)
    {
        return reinterpret_cast<Out>(in);
    }
    /*else if constexpr (reinterpret_convertible_to<In, uintptr_t>)
    {
        auto tmp = reinterpret_cast<uintptr_t>(in);
        return reinterpret_cast<Out>(tmp);
    }*/
    else
    {
        return reinterpret_cast<Out&>(reinterpret_cast<uintptr_t&>(in));
    }
}

template <typename T>
decltype(auto) _Deref(T* const ptr)
{
    if constexpr (std::is_pointer_v<T>)
        return *ptr;
    else
        return *_Force_cast<std::conditional_t<std::is_class_v<T> || std::is_member_function_pointer_v<T> || std::is_function_v<T>, void, T>**>(ptr);
}

template <size_t Count, typename T>
decltype(auto) _Deref(T* const ptr)
{
    const auto ptr1 = _Deref(ptr);
    if constexpr (Count == 1)
        return ptr1;
    else
        return _Deref<Count - 1>(ptr1);
}

template <typename From, typename T>
concept constructible_from = std::constructible_from<T, From>;

template <typename T>
constexpr size_t _Array_step() noexcept
{
    if constexpr (std::is_void_v<T>)
        return sizeof(uintptr_t);
    else
        return sizeof(T);
}

export namespace fd
{
    template <typename T>
    struct address
    {
        using value_type    = /*std::remove_const_t<T>*/ T;
        using pointer_type  = std::add_pointer_t<T>;
        using safe_out_type = std::conditional_t<std::is_class_v<T>, void, T>;

        union
        {
            uintptr_t value;
            pointer_type pointer;
        };

        address()
            : pointer(nullptr)
        {
        }

        address(std::nullptr_t)
            : value(0)
        {
        }

        template <std::integral Q>
        address(const Q val)
            : value(static_cast<uintptr_t>(val))
        {
        }

        template <typename Q>
        address(Q* ptr)
            : pointer(_Force_cast<pointer_type>(ptr))
        {
            static_assert(!std::is_class_v<value_type> || std::convertible_to<pointer_type, Q*>, __FUNCSIG__ ": unable to construct from pointer!");
        }

        template <typename Q>
        address(const address<Q> other)
            : address(other.pointer)
        {
        }

        //----

        bool operator!() const
        {
            return this->pointer == nullptr;
        }

        explicit operator bool() const
        {
            return this->pointer != nullptr;
        }

        //----

        /* #define ADDR_EQ_OP(_OP_)                                    \
            template <constructible_from<address> Q>          \
            auto FD_CONCAT(operator, _OP_)(const Q other) const     \
            {                                                       \
                return this->value _OP_ address(other).value; \
            }

        #define ADDR_MATH_OP(_OP_, _NAME_)                                                                     \
            template <constructible_from<address> Q>                                                     \
            address& FD_CONCAT(operator, _OP_, =)(const Q other)                                         \
            {                                                                                                  \
                static_assert(!std::is_class_v<value_type>, __FUNCSIG__ ": unable to change the class type!"); \
                this->value FD_CONCAT(_OP_, =)_Void_addr(other).value;                               \
                return *this;                                                                                  \
            }                                                                                                  \
            template <constructible_from<address> Q>                                                     \
            _Safe_addr FD_CONCAT(operator, _OP_)(const Q other) const                        \
            {                                                                                                  \
                return this->value _OP__Void_addr(other).value;                                      \
            }                                                                                                  \
            template <constructible_from<address> Q>                                                     \
            _Safe_addr _NAME_(const Q other) const                                           \
            {                                                                                                  \
                return this->value _OP__Void_addr(other).value;                                      \
            }

                ADDR_EQ_OP(<=>);
                ADDR_EQ_OP(==);

                ADDR_MATH_OP(+, plus);
                ADDR_MATH_OP(-, minus);
                ADDR_MATH_OP(*, multiply);
                ADDR_MATH_OP(/, divide); */

        //----

        using _Void_addr = address<const void>;
        using _Safe_addr = address<safe_out_type>;

        template <typename T>
        auto operator<=>(const T other) const requires(std::constructible_from<_Void_addr, T>)
        {
            return this->value <=> other.value;
        }

        template <typename T>
        bool operator==(const T other) const requires(std::constructible_from<_Void_addr, T>)
        {
            return this->value == _Void_addr(other).value;
        }

        template <typename T>
        _Safe_addr operator+(const T other) const requires(std::constructible_from<_Void_addr, T>)
        {
            return this->value + _Void_addr(other).value;
        }

        _Safe_addr plus(const _Void_addr other) const
        {
            return this->value + other.value;
        }

        template <typename T>
        _Safe_addr operator-(const T other) const requires(std::constructible_from<_Void_addr, T>)
        {
            return this->value - _Void_addr(other).value;
        }

        _Safe_addr minus(const _Void_addr other) const
        {
            return this->value - other.value;
        }

        template <typename T>
        _Safe_addr operator*(const T other) const requires(std::constructible_from<_Void_addr, T>)
        {
            return this->value * _Void_addr(other).value;
        }

        _Safe_addr multiply(const _Void_addr other) const
        {
            return this->value * other.value;
        }

        template <typename T>
        _Safe_addr operator/(const T other) const requires(std::constructible_from<_Void_addr, T>)
        {
            return this->value / _Void_addr(other).value;
        }

        _Safe_addr divide(const _Void_addr other) const
        {
            return this->value / other.value;
        }

        auto operator[](const ptrdiff_t index) const
        {
            auto tmp = *this;
            tmp.value += index * _Array_step<value_type>();
            return tmp.deref<1>();
        }

        pointer_type operator->() const
        {
            return this->pointer;
        }

        //----

        template <typename Q>
        operator Q() const requires(std::is_reference_v<Q>)
        {
            using ref_t = std::remove_reference_t<Q>;
            static_assert(!std::is_class_v<ref_t> || std::convertible_to<value_type, ref_t>);
            return *_Force_cast<ref_t*>(this->value);
        }

        template <typename Q>
        operator Q() const requires(std::is_pointer_v<Q> || std::is_member_function_pointer_v<Q> || std::is_function_v<Q>)
        {
            if constexpr (std::is_pointer_v<Q>)
                static_assert(std::constructible_from<address, Q>, __FUNCSIG__ ": unable to convert to pointer!");
            else
                static_assert(!std::is_class_v<value_type>, __FUNCSIG__ ": unable to convert to function pointer!");
            return _Force_cast<Q>(this->value);
        }

        template <typename Q>
        /*explicit*/
        operator Q() const requires(std::is_integral_v<Q>)
        {
            return static_cast<Q>(this->value);
        }

        //----

        template <size_t Count>
        auto deref() const
        {
            const auto ptr = _Deref<Count>(this->pointer);
            return address<decltype(ptr)>(ptr);
        }

        template <typename Q>
        /*[[deprecated]]*/ Q get() const
        {
            return _Force_cast<Q>(value);
        }

        //----

        _Safe_addr jmp(const ptrdiff_t offset) const
        {
            // Example:
            // E9 ? ? ? ?
            // The offset has to skip the E9 (JMP) instruction
            // Then deref the address coming after that to get to the function
            // Since the relative JMP is based on the next instruction after the address it has to be skipped

            // Base address is the address that follows JMP ( 0xE9 ) instruction
            _Void_addr base = this->value + offset;

            // Store the displacement
            // Note: Displacement address can be signed
            int32_t displacement = base.deref<1>();

            // The JMP is based on the instruction after the address
            // so the address size has to be added
            // Note: This is always 4 bytes, regardless of architecture
            base.value += sizeof(uint32_t);

            // Now finally do the JMP by adding the function address
            base.value += displacement;

            return base;
        }
    };

    template <typename T>
    address(T*) -> address<T>;
} // namespace fd
