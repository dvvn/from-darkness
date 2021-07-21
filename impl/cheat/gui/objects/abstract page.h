#pragma once
#include "renderable object.h"
#include "cheat/gui/tools/string wrapper.h"
#include "cheat/gui/widgets/selectable.h"

namespace cheat::gui::objects
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
				if constexpr (!std::is_default_constructible_v<T>)
				{
					BOOST_ASSERT("Unable to construct object!");
				}
				else
				{
					auto uptr = utl::make_unique<T>( );
					obj = static_cast<T*>(uptr.get( ));
					page__.emplace<0>(utl::move(uptr));
				}
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
		abstract_page(tools::string_wrapper&& name, T* obj = nullptr)
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
		T* init(tools::string_wrapper&& name, T* obj = nullptr)
		{
			name__.init(utl::move(name));
			return Init_this(obj);
		}

		const tools::string_wrapper& name( ) const;
		renderable_object* page( ) const;

		void render( );

	private:
		tools::string_wrapper_abstract name__;
		utl::variant<
			utl::unique_ptr<renderable_object>,
			utl::reference_wrapper<renderable_object>> page__;
	};

	class pages_storage_data final: public abstract_page, public widgets::selectable_internal
	{
	public:
		pages_storage_data(abstract_page&& page);

	protected:
		tools::string_wrapper::value_type Label( ) const override;
	};

	class abstract_pages_renderer: public renderable_object
	{
	public:
		void add_page(abstract_page&& page);

		virtual void init( );

		size_t pages_count()const;

	protected:
		utl::vector<pages_storage_data> pages_;
	};
}
