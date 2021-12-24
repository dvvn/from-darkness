module;

#include <array>
#include <ranges>
#include <functional>
#include <algorithm>
#include <variant>

export module cheat.csgo.math.array_view;

namespace rn = std::ranges;

export using std::array;
export using std::_Array_iterator;
export using std::_Array_const_iterator;

template<typename Arg1, typename ...Args>
constexpr decltype(auto) _Get_first_value(Arg1&& arg, Args&&...)
{
	return std::forward<Arg1>(arg);
}

export
{
	template<class T, size_t S>
	constexpr auto& _Array_unpack(const std::array<T, S>& arr) { return arr; }
	template<class T, size_t S>
	constexpr auto& _Array_unpack(std::array<T, S>& arr) { return arr; }
	template<class T, size_t S>
	constexpr decltype(auto) _Array_unpack(std::array<T, S>&& arr) { return std::move(arr); }

	template<typename T>
	concept array_unpackable = requires(T && val)
	{
		_Array_unpack(std::forward<T>(val));
	};
}

template<typename ValT, typename Arg>
constexpr bool _In_out_constructible( )
{
	if constexpr (is_inner_value_v<Arg>)
		return std::constructible_from<ValT, Arg::value_type>;
	else
		return std::constructible_from<ValT, Arg>;
}

template<typename T>
_INLINE_VAR constexpr bool is_tuple_v = false;
template<typename ...T>
_INLINE_VAR constexpr bool is_tuple_v<std::tuple<T...>> = true;

template<typename T>
_INLINE_VAR constexpr bool is_array_v = std::_Is_std_array_v<T> || std::is_bounded_array_v<T>;

constexpr auto _Flatten( ) noexcept { return std::tuple<>{}; }

template<class T, class... Tail>
constexpr auto _Flatten(T&& t, Tail&&... tail)
{
	constexpr auto unpack = []<typename Q>(Q && val)->decltype(auto)
	{
		if constexpr (array_unpackable<Q>)
			return _Array_unpack(std::forward<Q>(val));
		else
			return std::forward<Q>(val);
	};

	using raw_t = std::remove_cvref_t<T>;
	if constexpr (std::is_class_v<raw_t> || std::is_bounded_array_v<T>)
	{
		return std::tuple_cat(
			std::apply([]<typename ...Q>(Q&&... args) { return _Flatten(unpack(std::forward<Q>(args))...); }, unpack(t)), _Flatten(unpack(std::forward<Tail>(tail))...)
		);
	}
	else
	{
		return std::tuple_cat(std::forward_as_tuple(unpack(std::forward<T>(t))), _Flatten(unpack(std::forward<Tail>(tail))...));
	}
}

template<typename ...T>
struct smart_tuple :std::tuple<T...>
{
	constexpr smart_tuple(std::tuple<T...> tpl) :std::tuple<T...>(tpl) { }

	static constexpr bool size( ) { return sizeof...(T); }
};

//template<typename ...T>
//smart_tuple(std::tuple<T...>)->smart_tuple<std::tuple<T...>>;
//smart_tuple( )->smart_tuple<std::tuple<>>;

template<size_t OffsetDst = 0, size_t OffsetSrc = 0, class Dst, class Src, size_t ...I>
constexpr void _Construct(Dst& dst, Src& src, std::index_sequence<I...>)
{
	((std::get<I + OffsetDst>(dst) = std::get<I + OffsetSrc>(src)), ...);
}

template<size_t Offset, class Dst, class T, size_t ...I>
constexpr void _Fill(Dst& dst, T val, std::index_sequence<I...>)
{
	((std::get<I + Offset>(dst) = val), ...);
}

template<size_t DstOffset, size_t DstLimit, class Dst, class Src, size_t ...SrcI>
constexpr void _Fill_step(Dst& dst, Src& src, std::index_sequence<SrcI...> seq)
{
	if constexpr (DstOffset != DstLimit)
	{
		static_assert(DstOffset < DstLimit, "incorrect DstOffset");
		_Construct<DstOffset>(dst, src, seq);
		constexpr size_t next = DstOffset + seq.size( );
		static_assert(next <= DstLimit, "incorrect sequence");
		_Fill_step<next, DstLimit>(dst, src, seq);
	}
}

template<typename T>
struct min_max_result
{
	template<class Pair>
	constexpr min_max_result(Pair&& mmax) :min(mmax.first), max(mmax.second), diff(max - min)
	{
	}

	[[no_unique_address]] T min, max;
	[[no_unique_address]] std::remove_cvref_t<T> diff;
};

template<typename T>
min_max_result(std::pair<T, T>&)->min_max_result<T>;
template<typename T>
min_max_result(const std::pair<T, T>&)->min_max_result<T>;
template<typename T>
min_max_result(std::pair<T, T>&&)->min_max_result<std::remove_cvref_t<T>>;

export namespace cheat::csgo
{
	template<class T>
	struct array_view_default_value
	{
		using value_type = T;
		T value;

		constexpr array_view_default_value(T val) :value(val) { }
	};

	template<typename T>
	array_view_default_value(T&&)->array_view_default_value<std::remove_cvref_t<T>>;

	struct array_view_tag
	{
	};

	template<typename T>
	struct array_view_fill
	{
		T value;

		constexpr array_view_fill(T val) :value(val) { }
	};

#define ARRAY_VIEW_FILL(_TYPE_)\
	constexpr array_view_fill<_TYPE_> operator"" _fill(_TYPE_ val){ return val; }

	ARRAY_VIEW_FILL(long double);
	ARRAY_VIEW_FILL(unsigned long long);

	template<typename T>
	concept array_view_based = std::derived_from<std::remove_cvref_t<T>, array_view_tag>;

	//template<typename A, typename B>
	//concept array_view_equal = array_view_based<A> && array_view_based<B> && std::same_as<typename A::value_type, typename B::value_type> && A::_Size == B::_Size;

	template <class _Ty, size_t Size, array_view_default_value Default, class Base = std::array<_Ty, Size>>
	struct array_view :Base, array_view_tag
	{
		static constexpr size_t _Size = Size;

		using typename Base::value_type;

		/*constexpr array_view( ) :Base( )
		{
			fill(std::invoke(default_type{}));
		}*/

		//added for compatibility. Currently come code use this
		//array_view(_Ty) = delete;
		array_view(typename decltype(Default)::value_type) = delete;

		template<typename T>
		constexpr array_view(array_view_fill<T> f) : Base( )
		{
			fill(f.value);
		}

		template<typename ...Args>
		constexpr array_view(Args&&...args) : Base( )
		{
			smart_tuple dst = _Flatten(*static_cast<Base*>(this));
			smart_tuple src = _Flatten(std::forward<Args>(args)...);

			if constexpr (src.size( ) >= dst.size( ))
			{
				//copy visible src part
				_Construct(dst, src, std::make_index_sequence<dst.size( )>( ));
			}
			else
			{
				//copy src
				_Construct(dst, src, std::make_index_sequence<src.size( )>( ));
				fill<src.size( )>(Default.value);
			}
		}

		template<size_t Offset, typename T>
		constexpr void fill(T&& val, std::in_place_index_t<Offset> = std::in_place_index<Offset>)
		{
			smart_tuple dst = _Flatten(*static_cast<Base*>(this));
			smart_tuple src = _Flatten(std::forward<T>(val));

			_Fill_step<Offset, dst.size( )>(dst, src, std::make_index_sequence<src.size( )>( ));
		}

		template<typename T>
		constexpr void fill(T&& val)
		{
			fill<0>(std::forward<T>((val)));
		}
	};
}

namespace cheat::csgo
{
	template<typename ...T>
	using array_view_detect_type = std::remove_cvref_t<decltype(std::get<0>(_Flatten(_Get_first_value(std::declval<T>( )...))))>;

	template<typename ...T>
	_INLINE_VAR constexpr size_t array_view_detect_size = std::tuple_size_v<decltype(_Flatten(std::declval<T>( )...))>;

	template<typename ...T>
	constexpr auto array_view_make_default_value( )
	{
		using val_t = array_view_detect_type<T...>;
		array_view_default_value out = val_t{};
		return out;
	}
}

export namespace cheat::csgo
{
	template<typename ...T>
	array_view(T...)->array_view<array_view_detect_type<T...>, array_view_detect_size<T...>, array_view_make_default_value<T...>( )>;

	template<class R, class L>
	concept array_view_constructible = array_view_based<R> || std::constructible_from<std::remove_cvref_t<L>, R>;
}

namespace cheat::csgo
{
	enum array_view_operator :uint8_t
	{
		plus
		, minus
		, multiply
		, divide
	};

	template<array_view_operator Op, typename Dst, typename Src>
	constexpr void _AV_math_operator(Dst& dst, Src&& src)
	{
		if constexpr (Op == plus)
			dst += std::forward<Dst>(src);
		else if constexpr (Op == minus)
			dst -= std::forward<Dst>(src);
		else if constexpr (Op == multiply)
			dst *= std::forward<Dst>(src);
		else if constexpr (Op == divide)
			dst /= std::forward<Dst>(src);
	}

	template<size_t Offset, size_t Limit, size_t ...I>
	constexpr size_t _Get_next_offset(std::index_sequence<I...> seq)
	{
		if constexpr (Offset == Limit)
		{
			return Limit;
		}
		else
		{
			static_assert(Offset < Limit, "Incorrect Limit");
			constexpr auto next = Offset + seq.size( );
			static_assert(next <= Limit, "Incorrect sequence");
			return next;
		}
	}

	template<array_view_operator Op, size_t Offset, size_t Limit, typename Dst, typename Src, size_t ...I>
	constexpr void _AV_operator_apply(Dst& dst, Src&& src, std::index_sequence<I...> seq)
	{
		if constexpr (Offset != Limit)
		{
			constexpr auto next = _Get_next_offset<Offset, Limit>(seq);
			(_AV_math_operator<Op>(std::get<Offset + I>(dst), std::get<I>(src)), ...);
			_AV_operator_apply<Op, next, Limit>(dst, std::forward<Src>(src), seq);
		}
	}

	template<array_view_operator Op, array_view_based L, array_view_constructible<L> R>
	constexpr L& _AV_operator_selector(L& l, R&& r)
	{
#if 0
		if constexpr (rn::range<R>)
		{
			auto _L = rn::_Ubegin(l);
			auto _R = rn::_Ubegin(r);
			auto limit = std::min<size_t>(l.size( ), rn::distance(r));

			const auto dst = [&]( )->decltype(auto)
			{
				if constexpr (std::is_rvalue_reference_v<decltype(r)>)
					return std::move(*_R++);
				else
					return *_R++;
			};

			while (limit-- > 0)
				_AV_math_operator<Op>(*_L++, dst( ));
		}
		else if constexpr (std::is_arithmetic_v<L::value_type> && std::constructible_from<L::value_type, R>)
		{
			for (auto& src : l)
				_AV_math_operator<Op>(src, r);
		}
		else
		{
			L tmp = std::forward<R>(r);
			_AV_operator_selector<Op>(l, std::move(tmp));
		}

		return l;
#endif

		smart_tuple l_tpl = _Flatten(l);
		smart_tuple r_tpl = _Flatten(std::forward<R>(r));

		constexpr auto limit = std::min(l_tpl.size( ), r_tpl.size( ));
		_AV_operator_apply<Op, 0, limit>(l_tpl, r_tpl, std::make_index_sequence<limit>( ));

		return l;
	}

	template<array_view_operator Op, array_view_based L, array_view_constructible<L> R>
	constexpr L& _AV_operator_selector(L& l, const array_view_fill<R>& r)
	{
		smart_tuple l_tpl = _Flatten(l);
		smart_tuple r_tpl = _Flatten(r.value);

		_AV_operator_apply<Op, 0, l_tpl.size( )>(l_tpl, r_tpl, std::make_index_sequence<r_tpl.size( )>( ));

		return l;
	}
}

export namespace cheat::csgo
{
	template<array_view_based L, typename R>
	constexpr L& operator+=(L& l, R&& r)
	{
		return _AV_operator_selector<plus>(l, std::forward<R>(r));
	}

	template<array_view_based L, typename R>
	constexpr L& operator-=(L& l, R&& r)
	{
		return _AV_operator_selector<minus>(l, std::forward<R>(r));
	}

	template<array_view_based L, typename R>
	constexpr L& operator*=(L& l, R&& r)
	{
		return _AV_operator_selector<multiply>(l, std::forward<R>(r));
	}

	template<array_view_based L, typename R>
	constexpr L& operator/=(L& l, R&& r)
	{
		return _AV_operator_selector<divide>(l, std::forward<R>(r));
	}

	//---

	template<array_view_based L, typename R>
	constexpr L operator+(L l, R&& r)
	{
		return l += std::forward<R>(r);
	}

	template<array_view_based L, typename R>
	constexpr L operator-(L l, R&& r)
	{
		return l -= std::forward<R>(r);
	}

	template<array_view_based L, typename R>
	constexpr L operator*(L l, R&& r)
	{
		return l *= std::forward<R>(r);
	}

	template<array_view_based L, typename R>
	constexpr L operator/(L l, R&& r)
	{
		return l /= std::forward<R>(r);
	}

}

namespace cheat::csgo
{
	template<size_t Offset, size_t Limit, typename L, typename R, size_t ...Idx>
	constexpr bool _AV_equal_operator(const L& l, const R& r, std::index_sequence<Idx...> seq)
	{
		if constexpr (Offset == Limit)
		{
			static_assert(Offset > 0);
			return true;
		}
		else
		{
			constexpr auto next = _Get_next_offset<Offset, Limit>(seq);
			const bool equal = ((std::get<Idx>(l) == std::get<Idx>(r)) && ...);
			const bool equal_next = _AV_equal_operator<next, Limit>(l, r, seq);
			return equal && equal_next;
		}
	}

	template<typename L, typename R, size_t ...Idx>
	constexpr bool _AV_equal_operator(const L& l, const R& r, std::index_sequence<Idx...> seq)
	{
		constexpr bool can_memcmp = sizeof(L) == sizeof(R) &&
			(std::_Can_memcmp_elements<std::remove_cvref_t<decltype(std::get<Idx>(l))>, std::remove_cvref_t<decltype(std::get<Idx>(r))>> && ...);

		if constexpr (can_memcmp)
		{
			if (!std::is_constant_evaluated( ))
				return std::memcmp(std::addressof(l), std::addressof(r), sizeof(L));
		}

		return _AV_equal_operator<0, l.size( )>(l, r, seq);

		//return (std::get<Idx>(l) == std::get<Idx>(r) && ...);
	}

	template<array_view_based L, array_view_constructible<L> R>
	constexpr bool _AV_equal(const L& l, R&& r)
	{
		smart_tuple l_tpl = _Flatten(l);
		smart_tuple r_tpl = _Flatten(std::forward<R>(r));

		if constexpr (l_tpl.size( ) == r_tpl.size( ))
			return _AV_equal_operator(l_tpl, r_tpl, std::make_index_sequence<l_tpl.size( )>( ));
		else
			return false;
	}

	template<array_view_based L, array_view_constructible<L> R>
	constexpr bool _AV_equal(const L& l, const array_view_fill<R>& r)
	{
		smart_tuple l_tpl = _Flatten(l);
		smart_tuple r_tpl = _Flatten(std::forward<R>(r.value));

		return _AV_equal_operator<0, l_tpl.size( )>(l_tpl, r_tpl, std::make_index_sequence<r_tpl.size( )>( ));
	}
}

export namespace cheat::csgo
{
	template<array_view_based L, typename R>
	constexpr bool operator==(const L& l, R&& r)
	{
		return _AV_equal(l, std::forward<R>(r));
	}

	template<array_view_based L, typename R>
	constexpr bool operator!=(const L& l, R&& r)
	{
		return !(l == std::forward<R>(r));
	}

	/*constexpr array_view av = 1;
	constexpr auto aa = array_view{1,4} - 1;

	constexpr auto aa1 = [] {
		array_view a = 1;
		_AV_operator_selector<plus>(a, array_view{4});
		return a;
	}();*/

	//---

	template<array_view_based T>
	constexpr T operator-(T val)
	{
		for (auto& v : val)
			v = -v;
		return val;
	}

	//----helpers

	template<typename T, size_t Pos, size_t Index>
	class Array_view_item
	{
		std::array<T, Pos + 1> data_;
	public:

		constexpr Array_view_item( ) = delete;

		constexpr operator const T& ()const { return data_[Index]; }
		constexpr operator T& () { return data_[Index]; }

		Array_view_item& operator=(const T& value)
		{
			data_[Index] = value;
			return *this;
		}
	};

	static_assert(sizeof(Array_view_item<size_t, 0, 0>) == sizeof(size_t));
	static_assert(sizeof(Array_view_item<size_t, 1, 0>) == sizeof(size_t) * 2);
	static_assert(sizeof(Array_view_item<size_t, 2, 0>) == sizeof(size_t) * 3);

	//---


}