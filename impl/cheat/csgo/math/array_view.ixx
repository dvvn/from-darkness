module;

#include "array_view_includes.h"

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

export namespace cheat::csgo
{
	template<typename T>
	concept has_internal_data = requires(T val)
	{
		{ val._Data }->std::destructible;
	};
	//array_view_based<decltype(std::declval<T>( )._Data)>;

	struct unpack_helper
	{
		template<class T, size_t S>
		constexpr auto& operator()(const std::array<T, S>& arr) const
		{
			return arr;
		}
		template<class T, size_t S>
		constexpr auto& operator()(std::array<T, S>& arr) const
		{
			return arr;
		}
		template<class T, size_t S>
		constexpr decltype(auto) operator()(std::array<T, S>&& arr) const
		{
			return std::move(arr);
		}
	};

	constexpr auto _Flatten( ) noexcept { return std::tuple<>{}; }

	template<class T>
	constexpr decltype(auto) _Unpack(T&& t)
	{
		if constexpr (std::invocable<unpack_helper, T>)
		{
			return std::invoke(unpack_helper( ), std::forward<T>(t));
		}
		else if constexpr (has_internal_data<T>)
		{
			if constexpr (std::is_rvalue_reference_v<decltype(t)>)
				return _Unpack(std::move(t._Data));
			else
				return _Unpack(t._Data);
		}
		else
		{
			return std::forward<T>(t);
		}
	}

	template<class T, class... Tail>
	constexpr auto _Flatten(T&& t, Tail&&... tail)
	{
		using raw_t = std::remove_cvref_t<T>;
		if constexpr (std::is_class_v<raw_t> || std::is_bounded_array_v<T>)
		{
			return std::tuple_cat(
				std::apply([]<typename ...Q>(Q&&... args) { return _Flatten(_Unpack(std::forward<Q>(args))...); }, _Unpack(t))
				, _Flatten(_Unpack(std::forward<Tail>(tail))...)
			);
		}
		else
		{
			return std::tuple_cat(
				std::forward_as_tuple(std::forward<T>(t))
				, _Flatten(_Unpack(std::forward<Tail>(tail))...)
			);
		}
	}

	template<typename ...T>
	struct smart_tuple :std::tuple<T...>
	{
		constexpr smart_tuple(std::tuple<T...>&& tpl) :std::tuple<T...>(std::move(tpl)) { }

		static constexpr size_t size( ) { return sizeof...(T); }
	};


	/*template<typename ...T>
	smart_tuple(std::tuple<T...>)->smart_tuple<std::tuple<T...>>;
	smart_tuple( )->smart_tuple<std::tuple<>>;*/
}

template<typename Dst, typename Src>
constexpr void _Write_safe(Dst& dst, Src&& src)
{
	dst = Dst(std::forward<Src>(src));
}

template<size_t OffsetDst = 0, size_t OffsetSrc = 0, class Dst, class Src, size_t ...I>
constexpr void _Construct(Dst& dst, Src& src, std::index_sequence<I...>)
{
	(_Write_safe(std::get<I + OffsetDst>(dst), std::get<I + OffsetSrc>(src)), ...);
}

template<size_t Offset, class Dst, class T, size_t ...I>
constexpr void _Fill(Dst& dst, T val, std::index_sequence<I...>)
{
	(_Write_safe(std::get<I + Offset>(dst), val), ...);
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
		constexpr array_view_fill(T val) :value(val) { }

		constexpr array_view_fill operator-( )const
		{
			return -value;
		}

		T value;
	};

	struct array_view_empty
	{
	};

	template<typename T>
	inline constexpr bool is_fill_var_v = false;
	template<typename T>
	inline constexpr bool is_fill_var_v<array_view_fill<T>> = true;

#pragma warning(disable:4455)
	constexpr auto operator"" _fill(long double val) { return array_view_fill(val); }
	constexpr auto operator"" f_fill(long double val) { return array_view_fill(static_cast<float>(val)); }
	constexpr auto operator"" _fill(unsigned long long val) { return array_view_fill(val); }
#pragma warning(default:4455)

	template<typename T>
	concept array_view_based = std::derived_from<std::remove_cvref_t<T>, array_view_tag>;

	template<typename T>
	concept array_view_internally_based = has_internal_data<T> && array_view_based<decltype(std::declval<T>( )._Data)>;

	//template<typename A, typename B>
	//concept array_view_equal = array_view_based<A> && array_view_based<B> && std::same_as<typename A::value_type, typename B::value_type> && A::_Size == B::_Size;

	template<typename Av, typename A>
	constexpr bool _Bad_init_arg( )
	{
		static_assert(std::same_as<A, std::remove_cvref_t<A>>, "Wrong init arg check");
		using av_raw = std::remove_cvref_t<Av>;

		using default_t = decltype(std::declval<av_raw>( )._Default)::value_type;

		if constexpr (std::is_arithmetic_v<default_t>)
			return std::is_arithmetic_v<A>;
		else
			return std::same_as<default_t, A>;
	}

	template <class _Ty, size_t Size, array_view_default_value Default, class Base = std::array<_Ty, Size>>
	struct array_view :Base, array_view_tag
	{
		static constexpr size_t _Size = Size;
		static constexpr auto _Default = Default;

		using typename Base::value_type;

		template<typename ...Args>
		constexpr array_view(Args&&...args) : Base( )
		{
			constexpr auto args_count = sizeof...(Args);

			using first_args_t = std::remove_cvref_t<decltype(_Get_first_value(args..., nullptr))>;
			if constexpr (args_count == 1 && _Bad_init_arg<array_view, first_args_t>( ))
			{
				//Added for compatibility.
				//Currently some code looks like '{CLASS} = 1'; Must be replaced with '{CLASS} = 1_fill'
				static_assert(std::_Always_false<first_args_t>, __FUNCTION__": incompatible argument");
			}
			else if constexpr (args_count == 1 && is_fill_var_v<first_args_t>)
			{
				fill(args.value...);
			}
			else if constexpr (args_count == 1 && std::is_same_v<first_args_t, array_view_empty>)
			{
				//nothing here
			}
			else
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
					//copy whole src	
					if constexpr (src.size( ) > 0)
						_Construct(dst, src, std::make_index_sequence<src.size( )>( ));

					//write default value to others
					fill<src.size( )>(_Default.value);
				}
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
	inline constexpr size_t array_view_detect_size = std::tuple_size_v<decltype(_Flatten(std::declval<T>( )...))>;

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
	concept array_view_constructible = array_view_based<R> || (std::constructible_from<std::remove_cvref_t<L>, R> && !_Bad_init_arg<L, R>( ));
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
			dst += std::forward<Src>(src);
		else if constexpr (Op == minus)
			dst -= std::forward<Src>(src);
		else if constexpr (Op == multiply)
			dst *= std::forward<Src>(src);
		else if constexpr (Op == divide)
			dst /= std::forward<Src>(src);
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
		smart_tuple l_tpl = _Flatten(l);
		using r_raw = std::remove_cvref_t<R>;
		if constexpr (is_fill_var_v<r_raw>)
		{
			smart_tuple r_tpl = _Flatten(r.value);
			_AV_operator_apply<Op, 0, l_tpl.size( )>(l_tpl, r_tpl, std::make_index_sequence<r_tpl.size( )>( ));
		}
		else
		{
			//static_assert(!std::same_as<r_raw, array_view_empty>);
			smart_tuple r_tpl = _Flatten(std::forward<R>(r));
			constexpr auto limit = std::min(l_tpl.size( ), r_tpl.size( ));
			_AV_operator_apply<Op, 0, limit>(l_tpl, r_tpl, std::make_index_sequence<limit>( ));
		}

		return l;
	}

	//-----

	template<typename T>
	constexpr decltype(auto) _AV_unpack(T&& val)
	{
		if constexpr (/*array_view_internally_based*/has_internal_data<T>)
		{
			if constexpr (std::is_rvalue_reference_v<decltype(val)>)
				return std::move(val._Data);
			else
				return std::invoke([&]( )->auto& {return val._Data; });
		}
		else
		{
			return std::forward<T>(val);
		}
	}

	template<class T>
	concept array_view_unpackable = array_view_based<T> || array_view_internally_based<T>;

	template<array_view_operator Op, typename Lpx, typename Rpx>
	constexpr Lpx& _AV_operator_selector_proxy(Lpx& l, Rpx&& r)
	{
		auto& l_unpacked = _AV_unpack(l);
		_AV_operator_selector<Op>(l_unpacked, _AV_unpack(std::forward<Rpx>(r)));
		return l;
	}
}

export namespace cheat::csgo
{
	template<array_view_unpackable L, typename R>
	constexpr L& operator+=(L& l, R&& r)
	{
		return _AV_operator_selector_proxy<plus>(l, std::forward<R>(r));
	}

	template<array_view_unpackable L, typename R>
	constexpr L& operator-=(L& l, R&& r)
	{
		return _AV_operator_selector_proxy<minus>(l, std::forward<R>(r));
	}

	template<array_view_unpackable L, typename R>
	constexpr L& operator*=(L& l, R&& r)
	{
		return _AV_operator_selector_proxy<multiply>(l, std::forward<R>(r));
	}

	template<array_view_unpackable L, typename R>
	constexpr L& operator/=(L& l, R&& r)
	{
		return _AV_operator_selector_proxy<divide>(l, std::forward<R>(r));
	}

	//---

	template<array_view_unpackable L, typename R>
	constexpr L operator+(L l, R&& r)
	{
		return l += std::forward<R>(r);
	}

	template<array_view_unpackable L, typename R>
	constexpr L operator-(L l, R&& r)
	{
		return l -= std::forward<R>(r);
	}

	template<array_view_unpackable L, typename R>
	constexpr L operator*(L l, R&& r)
	{
		return l *= std::forward<R>(r);
	}

	template<array_view_unpackable L, typename R>
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
#if 0
		constexpr bool can_memcmp = sizeof(L) == sizeof(R) &&
			(std::_Can_memcmp_elements<std::remove_cvref_t<decltype(std::get<Idx>(l))>, std::remove_cvref_t<decltype(std::get<Idx>(r))>> && ...);

		if constexpr (can_memcmp)
		{
			if (!std::is_constant_evaluated( ))
				return std::memcmp(std::addressof(l), std::addressof(r), sizeof(L));
		}
#endif

		return _AV_equal_operator<0, l.size( )>(l, r, seq);
	}

	template<array_view_based L, array_view_constructible<L> R>
	constexpr bool _AV_equal(const L& l, R&& r)
	{
		smart_tuple l_tpl = _Flatten(l);
		using r_raw = std::remove_cvref_t<R>;
		if constexpr (is_fill_var_v<r_raw>)
		{
			smart_tuple r_tpl = _Flatten(std::forward<R>(r.value));
			return _AV_equal_operator<0, l_tpl.size( )>(l_tpl, r_tpl, std::make_index_sequence<r_tpl.size( )>( ));
		}
		else
		{
			//static_assert(!std::same_as<r_raw, array_view_empty>);
			smart_tuple r_tpl = _Flatten(std::forward<R>(r));
			return l_tpl.size( ) == r_tpl.size( ) && _AV_equal_operator(l_tpl, r_tpl, std::make_index_sequence<l_tpl.size( )>( ));
		}
	}

	template<class L, class R>
	constexpr bool _AV_equal_proxy(const L& l, R&& r)
	{
		return _AV_equal(_AV_unpack(l), _AV_unpack(std::forward<R>(r)));
	}
}

export namespace cheat::csgo
{
	template<array_view_unpackable L, typename R>
	constexpr bool operator==(const L& l, R&& r)
	{
		return _AV_equal_proxy(l, std::forward<R>(r));
	}

	template<array_view_unpackable L, typename R>
	constexpr bool operator!=(const L& l, R&& r)
	{
		return !(l == std::forward<R>(r));
	}

	//---

	template<array_view_unpackable T>
	constexpr T operator-(T val)
	{
		smart_tuple tpl = _Flatten(_AV_unpack(val));
		std::apply([]<typename Q>(Q & v) { v = -v; }, tpl);
		return val;
	}

	//----helpers

	template<typename T, size_t Elements, size_t Index>
	class Array_view_item
	{
		std::array<T, Elements + 1> data_;
	public:

		constexpr Array_view_item( ) = delete;

		constexpr operator const T& ()const { return data_[Index]; }
		constexpr operator T& () { return data_[Index]; }

		constexpr Array_view_item& operator=(const T& value)
		{
			data_[Index] = value;
			return *this;
		}
	};

	static_assert(sizeof(Array_view_item<size_t, 0, 0>) == sizeof(size_t));
	static_assert(sizeof(Array_view_item<size_t, 1, 0>) == sizeof(size_t) * 2);
	static_assert(sizeof(Array_view_item<size_t, 2, 0>) == sizeof(size_t) * 3);

	template<array_view_internally_based Base>
	class _Array_view_proxy :public Base
	{
		template<class T>
		static decltype(auto) _At(T& obj, size_t idx)
		{
			return obj
#ifdef _DEBUG
				.at(idx);
#else
				[idx];
#endif
		}

	public:

		using base_type = Base;

		using Base::Base;
		using Base::_Data;

		constexpr auto begin( ) { return _Data.begin( ); }
		constexpr auto begin( ) const { return _Data.begin( ); }
		constexpr auto end( ) { return _Data.end( ); }
		constexpr auto end( ) const { return _Data.end( ); }
		constexpr auto size( ) const { return _Data.size( ); }

		constexpr auto& operator[](size_t idx) { return _Data[idx]; }
		constexpr auto& operator[](size_t idx) const { return _Data[idx]; }
		constexpr auto& at(size_t idx) { return _At(_Data, idx); }
		constexpr auto& at(size_t idx) const { return _At(_Data, idx); }
	};
}