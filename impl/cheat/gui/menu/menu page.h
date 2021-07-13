#pragma once
#include "cheat/gui/renderable object.h"
#include "cheat/gui/_imgui extension/animated selectable.h"
#include "cheat/gui/_imgui extension/string wrapper.h"

namespace cheat::gui::menu
{
	using empty_page = renderable_object;

	class abstract_page //:public renderable_object
	{
	protected:
		template <typename T>
		T* Init_this(T* obj = nullptr)
		{
			if (obj)
			{
				page__.emplace<1>(*obj);
			}
			else
			{
				auto uptr = utl::make_unique<T>( );
				obj = static_cast<T*>(uptr.get( ));
				page__.emplace<0>(utl::move(uptr));
			}

			return obj;
		}

	public:
		abstract_page( ) = default;

		template <class T>
		abstract_page(T* obj = nullptr)
		{
			this->init(obj);
		}

		template <class T>
		abstract_page(imgui::string_wrapper&& name, T* obj = nullptr)
		{
			this->init(utl::move(name), obj);
		}

		template <class T>
		T* init(T* obj = nullptr)
		{
			auto obj2 = Init_this(obj);
			name__.init(obj2);
			return obj2;
		}

		template <class T>
		T* init(imgui::string_wrapper&& name, T* obj = nullptr)
		{
			name__.init(utl::move(name));
			return Init_this(obj);
		}

		const imgui::string_wrapper& name( ) const;
		renderable_object*           page( ) const;

		void render( );

	private:
		imgui::string_wrapper_abstract name__;
		utl::variant<utl::unique_ptr<renderable_object>,
					 utl::reference_wrapper<renderable_object>> page__;
	};

	struct pages_storage_data final: abstract_page, imgui::animated_selectable_base
	{
		pages_storage_data(abstract_page&& page);

	protected:
		imgui::string_wrapper::value_type Name( ) const override;
	};

	class abstract_pages_renderer: public renderable_object
	{
	public:
		void add_page(abstract_page&& page)
		{
#ifdef _DEBUG
			auto& name = page.name( );
			for (abstract_page& page_stored: objects_)
			{
				if (page_stored.name( ) == (name))
					BOOST_ASSERT("Duplicate detected!");
			}
#endif
			objects_.push_back((utl::move(page)));
		}

		virtual void init( )
		{
			object_selected_ = utl::addressof(objects_[0]);
		}

	protected:
		utl::vector<pages_storage_data> objects_;
		pages_storage_data*             object_selected_ = nullptr;
	};

	class page_with_tabs final: public abstract_pages_renderer
	{
	public:
		void render( ) override;
		void init( ) override;

	private:
		size_t chars_count__ = 0;
	};
}
