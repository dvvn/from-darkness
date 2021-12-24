#pragma once

#include <cmath>

#define ARRAY_VIEW_OPERATOR(_OP_,_CONCEPT_)\
	template<class Av, _CONCEPT_##<Av> R>\
	constexpr Av& operator##_OP_##=(Av& l, R&& r)\
	{\
		l._Data _OP_##= std::forward<R>(r);\
		return l;\
	}\
	template<class Av, _CONCEPT_##<Av> R>\
	constexpr Av operator##_OP_(Av l, R&& r)\
	{\
		l._Data _OP_##= std::forward<R>(r);\
		return l;\
	}

#define ARRAY_VIEW_OPERATOR2(_CONCEPT_)\
	template<class Av, _CONCEPT_##<Av> R>\
	constexpr bool operator==(const Av& l, R&& r)\
	{\
		return l._Data == std::forward<R>(r);\
	}\
	template<class Av, _CONCEPT_##<Av> R>\
	constexpr bool operator!=(const Av& l, R&& r)\
	{\
		return l._Data != std::forward<R>(r);\
	}

#define ARRAY_VIEW_OPERATORS(_CONCEPT_)\
	ARRAY_VIEW_OPERATOR(+,_CONCEPT_);\
	ARRAY_VIEW_OPERATOR(-,_CONCEPT_);\
	ARRAY_VIEW_OPERATOR(*,_CONCEPT_);\
	ARRAY_VIEW_OPERATOR(/,_CONCEPT_);\
	ARRAY_VIEW_OPERATOR2(_CONCEPT_);

export import cheat.csgo.math.array_view;

export
{
	

	template<typename Base>
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
		using Base::Base;
		using Base::_Data;

		constexpr auto& operator[](size_t idx) { return _Data[idx]; }
		constexpr auto& operator[](size_t idx) const { return _Data[idx]; }
		constexpr auto begin( ) { return _Data.begin( ); }
		constexpr auto begin( ) const { return _Data.begin( ); }
		constexpr auto end( ) { return _Data.end( ); }
		constexpr auto end( ) const { return _Data.end( ); }
		constexpr auto size( ) const { return _Data.size( ); }

		constexpr auto& at(size_t idx) { return _At(_Data, idx); }
		constexpr auto& at(size_t idx) const { return _At(_Data, idx); }
	};

	template<typename Base, typename ValT>
	struct _Array_view_proxy_math :Base
	{
		using Base::Base;
		using Base::_Data;

		ValT Length( ) const
		{
			return std::sqrt(LengthSqr( ));
		}
		constexpr ValT LengthSqr( ) const
		{
			ValT tmp = 0;
			for (auto&& v : _Data)
				tmp += v * v;
			return tmp;
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

	template<class T>
	constexpr auto& _Array_unpack(const _Array_view_proxy<T>& arr) { return arr._Data; }
	template<class T>
	constexpr auto& _Array_unpack(_Array_view_proxy<T>& arr) { return arr._Data; }
	template<class T>
	constexpr decltype(auto) _Array_unpack(_Array_view_proxy<T>&& arr) { return std::move(arr._Data); }

}
//#define ARRAY_VIEW_DATA_PROXY \
//	constexpr auto& operator[](size_t idx) { return _Data[idx]; }\
//	constexpr auto& operator[](size_t idx) const { return _Data[idx]; }\
//	constexpr auto begin( ) { return _Data.begin( ); }\
//	constexpr auto begin( ) const { return _Data.begin( ); }\
//	constexpr auto end( ) { return _Data.end( ); }\
//	constexpr auto end( ) const { return _Data.end( ); }\
//	constexpr auto size( ) const { return _Data.size( ); }

//#define ARRAY_VIEW_LENGTH \
//	value_type Length( ) const\
//	{\
//		return std::sqrt(LengthSqr( ));\
//	}\
//	constexpr value_type LengthSqr( ) const\
//	{\
//		value_type tmp = 0;\
//		for (auto v : *this)\
//			tmp += v * v;\
//		return tmp;\
//	}

//#define ARRAY_VIEW_DIST_TO \
//	value_type DistTo(const _This_type& other) const\
//	{\
//		const auto delta = *this - other;\
//		return delta.Length( );\
//	}\
//	value_type DistToSqr(const _This_type& other) const\
//	{\
//		const auto delta = *this - other;\
//		return delta.LengthSqr( );\
//	}

//#define ARRAY_VIEW_NORMALIZE \
//	_This_type Normalized( ) const\
//	{\
//		const auto l = Length( );\
//		if (l != 0)\
//			return *this / l;\
//		else\
//			return 0;\
//	}

//#define ARRAY_VIEW_DOT \
//	value_type Dot(const _This_type& other) const\
//	{\
//		value_type tmp = 0;\
//		for (size_t i = 0; i < _This_type::size();++i)\
//			tmp += (*this)[i] * other[i];\
//		return tmp;\
//	}