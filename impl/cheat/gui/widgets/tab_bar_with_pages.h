#pragma once

#include "tab_bar.h"

#include "cheat/gui/objects/shared_label.h"

#include <memory>

namespace cheat::gui::tools
{
	class perfect_string;
}

namespace cheat::gui::widgets
{
	class tab_bar_with_pages final: public objects::renderable
	{
	public:
		tab_bar_with_pages( );
		~tab_bar_with_pages( ) override;

		void render( ) override;

		tab_bar_item& add_item(tools::string_wrapper&& bar_name, const objects::renderable_shared& data);

#if 0
		template <std::derived_from<objects::abstract_label> B, std::derived_from<renderable> R>
		std::pair<B*, R*> add_item( )
		{
			auto bar_name_holder = std::make_shared<B>( );
			auto data_renderer   = std::make_shared<R>( );

			this->add_item(bar_name_holder, data_renderer);

			return {bar_name_holder.get( ), data_renderer.get( )};
		}

		template <std::derived_from<objects::abstract_label> B>
		B* add_item(const objects::renderable_shared& data)
		{
			auto bar_name_holder = std::make_shared<B>( );
			this->add_item(bar_name_holder, data);
			return bar_name_holder.get( );
		}

		template <std::derived_from<renderable> R>
		R* add_item(const objects::shared_label& bar_name)
		{
			auto data_renderer = std::make_shared<R>( );
			this->add_item(bar_name, data_renderer);
			return data_renderer.get( );
		}

		template <typename T>
			requires(std::derived_from<T, objects::abstract_label> && std::derived_from<T, renderable>)
		T* add_item(std::shared_ptr<T> obj = { })
		{
			if (!obj)
				obj = std::make_shared<T>( );

			this->add_item(obj, obj);
			return obj.get( );
		}
#endif

		tab_bar* operator->( ) const;

		//---

		renderable* get_item(tools::perfect_string&& title);

	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};
}
