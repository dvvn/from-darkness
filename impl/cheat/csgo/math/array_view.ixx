module;

#include <array>
#include <ranges>
#include <functional>
#include <algorithm>

export module cheat.csgo.math.array_view;

static constexpr bool _Check_args_count(intptr_t count, intptr_t limit)
{
	return count > 0 && count <= limit;
}

export namespace cheat::csgo
{
	template<std::default_initializable T>
	struct array_view_default_value
	{
		constexpr array_view_default_value( ) = default;
		constexpr T operator()( ) const { return T( ); }
	};

	struct array_view_tag
	{
	};

	template<typename T>
	concept array_view_based = std::derived_from<T, array_view_tag>;

	template<typename A, typename B>
	concept array_view_equal = array_view_based<A> && array_view_based<B> && std::same_as<typename A::value_type, typename B::value_type> && A::_Size == B::_Size;

	template <std::copyable _Ty, size_t Size, class Default = array_view_default_value<_Ty>, class Base = std::array<_Ty, Size>>
	struct array_view :Base, array_view_tag
	{
		using typename Base::value_type;
		static constexpr size_t _Size = Size;

		static constexpr bool _Is_simple = std::is_trivially_constructible_v<_Ty> && std::is_trivially_destructible_v<_Ty>;
		using _Ty_val = std::conditional_t<std::_Pass_functor_by_value_v<_Ty>, _Ty, const _Ty&>;

		static constexpr _Ty _Get_default_value( )
		{
			return std::invoke(Default{});
		}

		constexpr array_view(Base&& base) :Base(std::move(base))
		{
		}

		constexpr array_view(const Base& base) : Base(base)
		{
		}

		constexpr array_view(array_view&& base) : Base(std::move(base))
		{
		}

		constexpr array_view(const array_view& base) : Base(base)
		{
		}

		constexpr array_view( ) : Base( )
		{
			Base::fill(_Get_default_value( ));
		}

		template<typename Arg1, typename ...Args>
		constexpr array_view(Arg1&& arg, Args&&...args) :Base( )
		{
			static_assert(sizeof...(Args) + 1 <= _Size);
			static_assert(std::constructible_from<_Ty, Arg1>);
			static_assert((std::constructible_from<_Ty, Args> && ...));

			std::array tmp = {_Ty(std::forward<Arg1>(arg)),_Ty(std::forward<Args>(args))...};

			if constexpr (_Is_simple)
				std::ranges::copy(tmp, Base::begin( ));
			else
			{
				for (size_t idx = 0; idx < tmp.size( ); ++idx)
					(*this)[idx] = tmp[idx];
			}

			std::ranges::fill(Base::begin( ) + tmp.size( ), Base::end( ), _Get_default_value( ));
		}
	};

	template<typename Arg1, typename ...Args>
	array_view(Arg1, Args...)->array_view<Arg1, sizeof...(Args) + 1>;

	template<class L, class R>
		requires(array_view_equal<L, R>)
	constexpr L& operator+=(L& l, const R& r)
	{
		for (size_t idx = 0; idx < L::_Size; ++idx)
			l[idx] += r[idx];
		return l;
	}

	template<class L, class R>
		requires(array_view_equal<L, R>)
	constexpr L& operator-=(L& l, const R& r)
	{
		for (size_t idx = 0; idx < L::_Size; ++idx)
			l[idx] -= r[idx];
		return l;
	}


	template<class L, class R>
		requires(array_view_equal<L, R>)
	constexpr L operator+(L l, const R& r)
	{
		l += r;
		return l;
	}

	template<class L, class R>
		requires(array_view_equal<L, R>)
	constexpr L operator-(L l, const R& r)
	{
		l -= r;
		return l;
	}


	template<class L, class R>
		requires(array_view_equal<L, R>)
	constexpr L& operator*=(L& l, const R& r)
	{
		for (size_t idx = 0; idx < L::_Size; ++idx)
			l[idx] *= r[idx];
		return l;
	}

	template<class L, class R>
		requires(array_view_equal<L, R>)
	constexpr L& operator/=(L& l, const R& r)
	{
		for (size_t idx = 0; idx < L::_Size; ++idx)
			l[idx] /= r[idx];
		return l;
	}


	template<class L, class R>
		requires(array_view_equal<L, R>)
	constexpr L operator*(L l, const R& r)
	{
		l *= r;
		return l;
	}

	template<class L, class R>
		requires(array_view_equal<L, R>)
	constexpr L operator/(L l, const R& r)
	{
		l /= r;
		return l;
	}

	template<array_view_based T>
	constexpr T operator-(T val)
	{
		for (auto& v : val)
			v = -v;
		return val;
	}
}