module;

#include "vector_base_includes.h"

export module cheat.csgo.math:vector_base;
export import :array_view;

namespace cheat::csgo
{
	template<typename Out, typename T, size_t ...I>
	constexpr Out _Vector_length(const T& tpl, std::index_sequence<I...> seq)
	{
		constexpr auto get = [](Out val) {return val * val; };
		return (std::invoke(get, std::get<I>(tpl)) + ...);
	}
}

export namespace cheat::csgo
{
	template<typename Base, typename ValT>
	struct _Vector_math_base :Base
	{
		using Base::Base;
		using Base::_Data;

		ValT Length( ) const
		{
			return std::sqrt(LengthSqr( ));
		}
		constexpr ValT LengthSqr( ) const
		{
			smart_tuple tpl = _Flatten(*this);
			return _Vector_length<ValT>(tpl, std::make_index_sequence<tpl.size( )>( ));
		}

		ValT DistTo(const Base& other) const
		{
			const auto delta = _Data - other;
			return delta.Length( );
		}
		ValT DistToSqr(const Base& other) const
		{
			const auto delta = _Data - other;
			return delta.LengthSqr( );
		}

		Base Normalized( ) const
		{
			const auto l = Length( );
			if (l != 0)
				return _Data / l;
			else
				return 0_fill;
		}
	};
}