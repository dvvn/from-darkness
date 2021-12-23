#pragma once

#define ARRAY_VIEW_OPERATOR(_OP_,_CONCEPT_)\
	template<class Vb, _CONCEPT_##<Vb> R>\
	constexpr Vb& operator##_OP_##=(Vb& l, R&& r)\
	{\
		l._Data _OP_##= std::forward<R>(r);\
		return l;\
	}\
	template<class Vb, _CONCEPT_##<Vb> R>\
	constexpr Vb operator##_OP_(Vb l, R&& r)\
	{\
		l._Data _OP_##= std::forward<R>(r);\
		return l;\
	}

#define ARRAY_VIEW_OPERATOR2(_CONCEPT_)\
	template<class Vb, _CONCEPT_##<Vb> R>\
	constexpr bool operator==(const Vb& l, R&& r)\
	{\
		return l._Data == std::forward<R>(r);\
	}\
	template<class Vb, _CONCEPT_##<Vb> R>\
	constexpr bool operator!=(const Vb& l, R&& r)\
	{\
		return l._Data != std::forward<R>(r);\
	}

#define ARRAY_VIEW_DATA_PROXY \
	constexpr auto& operator[](size_t idx) { return _Data[idx]; }\
	constexpr auto& operator[](size_t idx) const { return _Data[idx]; }\
	constexpr auto begin( ) { return _Data.begin( ); }\
	constexpr auto begin( ) const { return _Data.begin( ); }\
	constexpr auto end( ) { return _Data.end( ); }\
	constexpr auto end( ) const { return _Data.end( ); }\
	constexpr auto size( ) const { return _Data.size( ); }

#define ARRAY_VIEW_LENGTH \
	value_type Length( ) const\
	{\
		return std::sqrt(LengthSqr( ));\
	}\
	constexpr value_type LengthSqr( ) const\
	{\
		value_type tmp = 0;\
		for (auto v : *this)\
			tmp += v * v;\
		return tmp;\
	}

#define ARRAY_VIEW_DIST_TO \
	value_type DistTo(const _This_type& other) const\
	{\
		const auto delta = *this - other;\
		return delta.Length( );\
	}\
	value_type DistToSqr(const _This_type& other) const\
	{\
		const auto delta = *this - other;\
		return delta.LengthSqr( );\
	}

#define ARRAY_VIEW_NORMALIZE \
	_This_type Normalized( ) const\
	{\
		const auto l = Length( );\
		if (l != 0)\
			return *this / l;\
		else\
			return 0;\
	}

#define ARRAY_VIEW_DOT \
	value_type Dot(const _This_type& other) const\
	{\
		value_type tmp = 0;\
		for (size_t i = 0; i < _This_type::size();++i)\
			tmp += (*this)[i] * other[i];\
		return tmp;\
	}