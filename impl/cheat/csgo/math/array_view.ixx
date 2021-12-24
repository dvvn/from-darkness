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
static constexpr decltype(auto) _Get_first_value(Arg1&& arg, Args&&...)
{
	return std::forward<Arg1>(arg);
}

//template<class>
//static constexpr void _Std_array_based( ) { }

template<class T, size_t S>
static constexpr auto& _Array_unpack(const std::array<T, S>& arr) { return arr; }
template<class T, size_t S>
static constexpr auto& _Array_unpack(std::array<T, S>& arr) { return arr; }
template<class T, size_t S>
static constexpr decltype(auto) _Array_unpack(std::array<T, S>&& arr) { return std::move(arr); }

template<typename T>
concept array_unpackable = requires(T val)
{
	_Array_unpack(val);
};

//template <class Arr>
//	requires(!std::is_void_v<decltype(_Std_array_based(std::declval<Arr>( )))>)
//struct std::tuple_size<Arr>
//{
//	static constexpr size_t value = sizeof(Arr) / sizeof(rn::range_value_t<Arr>);
//};

template<typename ValT, typename Arg>
static constexpr bool _In_out_constructible( )
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

constexpr auto flatten( ) noexcept { return std::tuple<>{}; }

template<class T, class... Tail>
constexpr auto flatten(T&& t, Tail&&... tail)
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
			std::apply([]<typename ...Q>(Q&&... args) { return flatten(unpack(std::forward<Q>(args))...); }, unpack(t)), flatten(unpack(std::forward<Tail>(tail))...)
		);
	}
	else
	{
		return std::tuple_cat(std::forward_as_tuple(std::forward<T>(t)), flatten(unpack(std::forward<Tail>(tail))...));
	}
}


template<size_t OffsetDst = 0, size_t OffsetSrc = 0, class Dst, class Src, size_t ...I>
static constexpr void _Construct(Dst& dst, Src& src, std::index_sequence<I...>)
{
	((std::get<I + OffsetDst>(dst) = std::get<I + OffsetSrc>(src)), ...);
}

template<size_t Offset, class Dst, class T, size_t ...I>
static constexpr void _Fill(Dst& dst, T val, std::index_sequence<I...>)
{
	((std::get<I + Offset>(dst) = val), ...);
}

template<size_t Offset, class Dst, class Src, size_t ...SrcI>
static constexpr void _Fill_step(Dst& dst, Src& src, std::index_sequence<SrcI...> seq)
{
	constexpr auto pos = Offset - seq.size( );

	if constexpr (pos >= 0)
		_Construct<pos>(dst, src, seq);
	if constexpr (pos > 0)
		_Fill_step<pos>(dst, src, seq);

}

template<typename T>
struct min_max_result
{
	constexpr min_max_result(T min, T max) :min(min), max(max), diff(max - min)
	{
	}

	[[no_unique_address]] T min, max, diff;
};

template<typename T>
static constexpr auto _Min_max(T l, T r)
{
	auto [min, max] = std::minmax(l, r);
	return min_max_result{min,max};
}

export namespace cheat::csgo
{
	template<class T>
	struct array_view_default_value
	{
		constexpr array_view_default_value( ) = default;
		constexpr auto operator()( ) const
		{
			T tmp = {};
			return std::get<0>(flatten(std::move(tmp)));
		}
	};

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

	template <class _Ty, size_t Size, class Default = array_view_default_value<_Ty>, class Base = std::array<_Ty, Size>>
	struct array_view :Base, array_view_tag
	{
		static constexpr size_t _Size = Size;

		using typename Base::value_type;
		using default_type = Default;

		/*constexpr array_view( ) :Base( )
		{
			fill(std::invoke(default_type{}));
		}*/

		template<typename T>
		constexpr array_view(array_view_fill<T> f) : Base( )
		{
			fill(f.value);
		}

		template<typename ...Args>
		constexpr array_view(Args&&...args) : Base( )
		{
			auto dst = flatten(*static_cast<Base*>(this));
			auto src = flatten(std::forward<Args>(args)...);
			constexpr auto m = _Min_max(std::tuple_size_v<decltype(dst)>, std::tuple_size_v<decltype(src)>);

			if constexpr (m.min > 0)
				_Construct(dst, src, std::make_index_sequence<m.min>( ));

			if constexpr (m.diff > 0)
				_Fill<m.min>(dst, std::invoke(default_type{}), std::make_index_sequence<m.diff>( ));
		}

		template<typename T>
		constexpr void fill(T&& val)
		{
			auto dst = flatten(*static_cast<Base*>(this));
			auto src = flatten(std::forward<T>(val));

			constexpr auto dst_s = std::tuple_size_v<decltype(dst)>;
			constexpr auto src_s = std::tuple_size_v<decltype(src)>;

			_Fill_step<dst_s>(dst, src, std::make_index_sequence<src_s>( ));
		}
	};

	template<typename ...T>
	array_view(T...)->array_view<
		std::remove_cvref_t<decltype(std::get<0>(flatten(_Get_first_value(std::declval<T>( )...))))>
		, std::tuple_size_v<decltype(flatten(std::declval<T>( )...))>>;

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

	template<array_view_operator Op, typename Src, typename Dst>
	static constexpr void _AV_operator_impl(Src& src, Dst&& dst)
	{
		if constexpr (Op == plus)
			src += std::forward<Dst>(dst);
		else if constexpr (Op == minus)
			src -= std::forward<Dst>(dst);
		else if constexpr (Op == multiply)
			src *= std::forward<Dst>(dst);
		else if constexpr (Op == divide)
			src /= std::forward<Dst>(dst);
	}

	template<array_view_operator Op, array_view_based L, array_view_constructible<L> R>
	static constexpr L& _AV_operator_selector(L& l, R&& r)
	{
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
				_AV_operator_impl<Op>(*_L++, dst( ));
		}
		else if constexpr (std::is_arithmetic_v<L::value_type> && std::constructible_from<L::value_type, R>)
		{
			for (auto& src : l)
				_AV_operator_impl<Op>(src, r);
		}
		else
		{
			L tmp = std::forward<R>(r);
			_AV_operator_selector<Op>(l, std::move(tmp));
		}

		return l;
	}
}

export namespace cheat::csgo
{
	template<array_view_based L, array_view_constructible<L> R>
	constexpr L& operator+=(L& l, R&& r)
	{
		return _AV_operator_selector<plus>(l, std::forward<R>(r));
	}

	template<array_view_based L, array_view_constructible<L> R>
	constexpr L& operator-=(L& l, R&& r)
	{
		return _AV_operator_selector<minus>(l, std::forward<R>(r));
	}

	template<array_view_based L, array_view_constructible<L> R>
	constexpr L& operator*=(L& l, R&& r)
	{
		return _AV_operator_selector<multiply>(l, std::forward<R>(r));
	}

	template<array_view_based L, array_view_constructible<L> R>
	constexpr L& operator/=(L& l, R&& r)
	{
		return _AV_operator_selector<divide>(l, std::forward<R>(r));
	}

	//---

	template<array_view_based L, array_view_constructible<L> R>
	constexpr L operator+(L l, R&& r)
	{
		return l += std::forward<R>(r);
	}

	template<array_view_based L, array_view_constructible<L> R>
	constexpr L operator-(L l, R&& r)
	{
		return l -= std::forward<R>(r);
	}

	template<array_view_based L, array_view_constructible<L> R>
	constexpr L operator*(L l, R&& r)
	{
		return l *= std::forward<R>(r);
	}

	template<array_view_based L, array_view_constructible<L> R>
	constexpr L operator/(L l, R&& r)
	{
		return l /= std::forward<R>(r);
	}

	//----

	template<array_view_based L, array_view_constructible<L> R>
	constexpr bool operator==(const L& l, R&& r)
	{
		if constexpr (array_view_based<R> && std::same_as<rn::range_value_t<L>, rn::range_value_t<R>>)
			return rn::equal(l, std::forward<R>((r)));
		else
			return l == L(std::forward<R>(r));
	}

	template<array_view_based L, array_view_constructible<L> R>
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

	template<size_t Number, typename T>
	class Array_view_item
	{
		std::array<uint8_t, (Number == 0 ? 0 : sizeof(T) * (Number - 1))> pad_;
		T val_;

	public:

		constexpr Array_view_item( ) = default;

		constexpr operator const T& ()const { return val_; }
		constexpr operator T& () { return val_; }
	};

	static_assert(sizeof(Array_view_item<__LINE__, size_t>) == sizeof(size_t) * __LINE__);
}