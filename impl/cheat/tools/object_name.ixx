module;

#include <nstd/type name.h>
#include <nstd/core.h>

export module cheat.tools.object_name;

#define DROP_NAMESPACE_HOLDER_FN(_NAME_,_NSPACE_)\
inline constexpr auto _NAME_##_holder = []\
{\
	constexpr auto raw = nstd::detail::type_name_impl<T>( );\
	constexpr auto buffer = nstd::drop_namespace(raw, NSTD_STRINGIZE(_NSPACE_));\
	if constexpr (buffer.ideal( ))\
		return buffer;\
	else\
		return buffer.make_ideal<buffer.str_size>( );\
}();

#define DROP_NAMESPACE_GETTER(_NAME_)\
template <class T>\
constexpr auto _NAME_( )\
{\
	return _NAME_##_holder<T>.view( );\
};\
template <template<class...>class T>\
constexpr auto _NAME_( )\
{\
	return _NAME_##partial_holder<T>.view( );\
};

#define DROP_NAMESPACE_HOLDER(_NAME_,_NSPACE_)\
template <class T>\
DROP_NAMESPACE_HOLDER_FN(_NAME_, _NSPACE_);\
template <template<class...>class T>\
DROP_NAMESPACE_HOLDER_FN(_NAME_##_partial, _NSPACE_);

//---

DROP_NAMESPACE_HOLDER(object_name, cheat);
DROP_NAMESPACE_HOLDER(csgo_object_name, cheat);

export namespace cheat::inline tools
{
	DROP_NAMESPACE_GETTER(object_name);
	DROP_NAMESPACE_GETTER(csgo_object_name);
}