module;

#include <array>
#include <ranges>
#include <functional>
#include <algorithm>
#include <variant>

#include <nstd/type_traits.h>

export module cheat.csgo.math.array_view;

namespace rn = std::ranges;

//struct args_count_getter
//{
//	template<typename T>
//		requires(std::is_arithmetic_v<T>)
//	constexpr size_t operator()(T)const
//	{
//		return 1;
//	}
//	template<typename T, size_t ...I>
//	constexpr size_t operator()(const T& obj, std::index_sequence<I...>)const
//	{
//		return (std::invoke(*this, std::get<I>(obj)) + ...);
//	}
//
//	template<typename ...Args>
//	constexpr size_t operator()(const std::tuple<Args...>& tpl)const
//	{
//		return std::invoke(*this, tpl, std::index_sequence_for<Args...>( ));
//	}
//
//	template<typename T, size_t Size>
//	constexpr size_t operator()(const std::array<T, Size>& arr)const
//	{
//		return std::invoke(*this, arr[0]) * Size;
//	}
//
//	template<typename ...Args>
//	constexpr size_t operator()(const Args&...args)const
//	{
//		return (std::invoke(*this, args) + ...);
//	}
//};
//
//_INLINE_VAR constexpr auto _Get_args_count = args_count_getter( );

template<typename Arg1 = void, typename ...Args>
struct first_arg
{
	using type = Arg1;
};

template<typename T>
struct inner_value //for ranges
{
	using value_type = T;
	T val;
};

template<typename T>
_INLINE_VAR constexpr bool is_inner_value_v = false;

template<typename T>
_INLINE_VAR constexpr bool is_inner_value_v<inner_value<T>> = true;

template<typename Arg1, typename ...Args>
static constexpr decltype(auto) _Get_first_value(Arg1&& arg, Args&&...)
{
	using val_t = std::remove_cvref_t<Arg1>;
	if constexpr (is_inner_value_v<val_t>)
		return std::forward<val_t::value_type>(arg.val);
	else
		return std::forward<Arg1>(arg);
}

template<typename ValT, typename Arg>
static constexpr bool _In_out_constructible( )
{
	if constexpr (is_inner_value_v<Arg>)
		return std::constructible_from<ValT, Arg::value_type>;
	else
		return std::constructible_from<ValT, Arg>;
}

template<typename ...T>
_INLINE_VAR constexpr bool is_tuple_v = false;

template<typename ...T>
_INLINE_VAR constexpr bool is_tuple_v<std::tuple<T...>> = true;

template<typename Itr>
class partial_filler
{
	Itr begin_, end_;
public:
	using value_type = std::iter_value_t<Itr>;

	constexpr partial_filler(Itr begin, Itr end) :begin_(begin), end_(end)
	{
	}

	template<class T>
	constexpr partial_filler(T& obj) : partial_filler(rn::_Ubegin(obj), rn::_Uend(obj))
	{
	}

	constexpr Itr begin( )const { return begin_; }
	constexpr Itr end( )const { return end_; }

	template<typename T, size_t ...I>
		requires(is_tuple_v<std::remove_cvref_t<T>>)
	constexpr void operator()(T&& tpl, std::index_sequence<I...>)
	{
		std::invoke(*this, inner_value{std::get<I>(std::forward<T>(tpl))}...);
	}

	template<typename T>
		requires(is_tuple_v<std::remove_cvref_t<T>>)
	constexpr void operator()(T&& tpl)
	{
		//std::apply(*this,std::forward<T>(tpl));
		std::invoke(*this, std::forward<T>(tpl), std::make_index_sequence<std::tuple_size_v<T>>( ));
	}

	template<rn::range Rng>
	constexpr void operator()(Rng&& rng)
	{
		constexpr bool rvalue = std::is_rvalue_reference_v<decltype(rng)>;
		using rng_val = rn::range_value_t<Rng>;
		if constexpr (std::constructible_from<rng_val, value_type>)
		{
			if constexpr (rvalue)
				rn::move(rng, begin_);
			else
				rn::copy(rng, begin_);
		}
		else
		{
			for (auto& v : rng)
			{
				if constexpr (rvalue)
					std::invoke(*this, inner_value(std::move(v)));
				else
					std::invoke(*this, inner_value(v));
			}
		}
		begin_ += rn::distance(rng);
	}

	template<typename ...Args>
	constexpr void operator()(Args&&...args)
	{
		constexpr size_t args_count = sizeof...(Args);
		using first_raw_t = first_arg<Args...>::type;
		using first_t = std::remove_cvref_t<first_raw_t>;
		if constexpr (args_count == 0)
		{
			//nothing
		}
		else if constexpr (args_count == 1 && _In_out_constructible<value_type, first_t>( ))
		{
			decltype(auto) first = _Get_first_value(std::forward<Args>(args)...);
			if constexpr (is_inner_value_v<first_t>)
			{
				*begin_++ = std::forward<decltype(first)>(first);
			}
			else
			{
				rn::fill(begin_, end_, first);
				begin_ = end_;
			}
		}
		else if constexpr (std::_All_same<std::remove_cvref_t<Args>...>::value)
		{
			std::invoke(*this, std::array{std::forward<Args>(args)...});
		}
		else
		{
			(std::invoke(*this, std::forward<Args>(args)), ...);
		}
	}
};

template<rn::range Rng>
partial_filler(Rng& rng)->partial_filler<decltype(rn::_Ubegin(rng))>;

//template<typename Def, nstd::has_array_access T, typename ...Args>
//	requires(sizeof...(Args) > 0 && _Get_args_count(T( )) >= _Get_args_count(Args( )...) && std::invocable<partial_filler<rn::iterator_t<T>>, Args...>)
//static constexpr void _AV_construct(Def def, T& in, Args&&...vals)
//{
//	partial_filler filler = in.begin_( );
//	std::invoke(filler, std::forward<Args>(vals)...);
//	rn::fill(filler.itr, in.end( ), def);
//}

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
	concept array_view_based = std::derived_from<std::remove_cvref_t<T>, array_view_tag>;

	//template<typename A, typename B>
	//concept array_view_equal = array_view_based<A> && array_view_based<B> && std::same_as<typename A::value_type, typename B::value_type> && A::_Size == B::_Size;

	template <std::copyable _Ty, size_t Size, class Default = array_view_default_value<_Ty>, class Base = std::array<_Ty, Size>>
	struct array_view :Base, array_view_tag
	{
		static constexpr size_t _Size = Size;

		using typename Base::value_type;
		using default_type = Default;

		constexpr array_view( ) :Base( )
		{
			auto def = std::invoke(Default{});
			Base::fill(def);
		}

		template<typename ...Args>
		constexpr array_view(Args&&...args) : Base( )
		{
			auto def = std::invoke(Default{});
			partial_filler filler = *this;
			std::invoke(filler, std::forward<Args>(args)...);
			rn::fill(filler, def);
		}
	};

	template<typename Arg1, typename ...Args>
	array_view(Arg1, Args...)->array_view<Arg1, sizeof...(Args) + 1>;

	constexpr array_view<float, 4> test = std::tuple{2};

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

	template<array_view_operator Op, typename Out, typename In>
	static constexpr void _AV_operator_impl(Out& out, In&& in)
	{
		if constexpr (Op == plus)
			out += std::forward<In>(in);
		else if constexpr (Op == minus)
			out -= std::forward<In>(in);
		else if constexpr (Op == multiply)
			out *= std::forward<In>(in);
		else if constexpr (Op == divide)
			out /= std::forward<In>(in);
	}

	template<array_view_operator Op, array_view_based L, array_view_constructible<L> R>
	static constexpr L& _AV_operator_selector(L& l, R&& r)
	{
		if constexpr (rn::range<R>)
		{
			auto _L = rn::_Ubegin(l);
			auto _R = rn::_Ubegin(r);
			auto limit = std::min<size_t>(l.size( ), rn::distance(r));

			const auto in = [&]( )->decltype(auto)
			{
				if constexpr (std::is_rvalue_reference_v<decltype(r)>)
					return std::move(*_R++);
				else
					return *_R++;
			};

			while (limit-- > 0)
				_AV_operator_impl<Op>(*_L++, in( ));
		}
		else if constexpr (std::is_arithmetic_v<L::value_type> && std::constructible_from<L::value_type, R>)
		{
			for (auto& out : l)
				_AV_operator_impl<Op>(out, r);
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

	template<size_t Number>
	class Array_view_item
	{
		std::array<uint8_t, sizeof(float)* (Number == 0 ? 0 : Number - 1)> pad_;
		float val_;

	public:
		constexpr operator const float& ()const { return val_; }
		constexpr operator float& () { return val_; }
	};

	static_assert(sizeof(Array_view_item<__LINE__>) == sizeof(float) * __LINE__);
}