#pragma once
#include "cheat/gui/objects/renderable object.h"

#include <tuple>

namespace cheat::gui::widgets
{
	/*template <typename ...Ts>
	class multi_widget;

	template <typename ...>
	_INLINE_VAR constexpr bool is_multi_viedget_v = false;

	template <typename ...T>
	_INLINE_VAR constexpr bool is_multi_viedget_v<multi_widget<T...>> = true;*/

	template <typename ...Ts>
		requires(std::derived_from<Ts, objects::renderable> && ...)
	class multi_widget final: public objects::renderable, public std::tuple<Ts...>
	{
		template <size_t ...I>
		void render_impl(std::index_sequence<I...>)
		{
			(static_cast<renderable&>(std::get<I>(*this)).render( ), ...);
		}

	public:

		template <class ...Q>
		constexpr multi_widget(Q&& ...ts) : std::tuple<Ts...>(std::forward<Q>(ts)...)
		{
		}

		void render( ) override
		{
			this->render_impl(std::index_sequence_for<Ts...>( ));
		}
	};

	template <class ...Ts>
	multi_widget(Ts&&...) -> multi_widget<std::remove_reference_t<Ts>...>;
}
