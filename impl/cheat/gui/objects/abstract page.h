#pragma once

#if 0
#include "renderable object.h"
#include "cheat/gui/tools/string wrapper.h"
#include "cheat/gui/widgets/selectable.h"

#include <memory>
#include <vector>

namespace cheat::gui::objects
{
	class abstract_page //:public renderable_object
	{
	public:
		abstract_page( ) = default;

		template <typename ...Ts>
		abstract_page(Ts&& ...args)
		{
			init(std::forward<Ts>(args)...);
		}

		template <std::default_initializable T>
		T* init( )
		{
			auto uptr = std::make_unique<T>( );
			auto obj  = static_cast<T*>(uptr.get( ));
			page__.emplace<unique_page>(std::move(uptr));
			name__.init(obj);

			return obj;
		}

		template <class T>
		T* init(T* obj)
		{
			page__.emplace<ref_page>(std::ref(*obj));
			name__.init(obj);

			return obj;
		}

		template <class T>
		T* init(const std::shared_ptr<T>& obj)
		{
			page__.emplace<shared_page>(obj);
			name__.init(obj.get( ));

			return obj.get( );
		}

		template <std::invocable T>
		auto init(T&& fn)
		{
			return init(std::invoke(fn));
		}

		template <class T>
		T* init(tools::string_wrapper&& name)
		{
			auto uptr = std::make_unique<T>( );
			auto obj  = static_cast<T*>(uptr.get( ));
			page__.emplace<unique_page>(std::move(uptr));
			name__.init(std::move(name));

			return obj;
		}

		template <class T>
		T* init(tools::string_wrapper&& name, T* obj)
		{
			page__.emplace<ref_page>(std::ref(*obj));
			name__.init(std::move(name));

			return obj;
		}

		template <class T>
		T* init(tools::string_wrapper&& name, const std::shared_ptr<T>& obj)
		{
			page__.emplace<shared_page>(obj);
			name__.init(std::move(name));

			return obj.get( );
		}

		template <std::invocable T>
		auto init(tools::string_wrapper&& name, T&& fn)
		{
			return init(std::move(name), std::invoke(fn));
		}

		const tools::string_wrapper& name( ) const;
		renderable*                  page( ) const;

		void render( );

	private:
		using unique_page = std::unique_ptr<renderable>;
		using shared_page = std::shared_ptr<renderable>;
		using ref_page = std::reference_wrapper<renderable>;

		std::variant<unique_page, shared_page, ref_page> page__;
		tools::string_wrapper_abstract                   name__;
	};

	class pages_storage_data final: public abstract_page, public widgets::selectable_internal
	{
	public:
		pages_storage_data(abstract_page&& page);

	protected:
		tools::string_wrapper::value_type Label( ) const override;
	};

	class abstract_pages_renderer: public renderable
	{
	public:
		void add_page(abstract_page&& page);

		virtual void init( );

		size_t pages_count( ) const;

	protected:
		std::vector<pages_storage_data> pages_;
	};
}
#endif