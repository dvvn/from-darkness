#pragma once
#include "renderable object.h"
#include "cheat/gui/tools/string wrapper.h"
#include "cheat/gui/widgets/selectable.h"

namespace cheat::gui::objects
{
	using empty_page = renderable_object;

	class abstract_page //:public renderable_object
	{
	public:
		abstract_page( ) = default;

		template <typename ...Ts>
		abstract_page(Ts&& ...args)
		{
			init(utl::forward<Ts>(args)...);
		}

		template <std::default_initializable T>
		T* init( )
		{
			auto uptr = utl::make_unique<T>( );
			auto obj  = static_cast<T*>(uptr.get( ));
			page__.emplace<unique_page>(utl::move(uptr));
			name__.init(obj);

			return obj;
		}

		template <class T>
		T* init(T* obj)
		{
			page__.emplace<ref_page>(utl::ref(*obj));
			name__.init(obj);

			return obj;
		}

		template <class T>
		T* init(const utl::shared_ptr<T>& obj)
		{
			page__.emplace<shared_page>(obj);
			name__.init(obj.get( ));

			return obj.get( );
		}

		template <std::invocable T>
		auto init(T&& fn)
		{
			return init(utl::invoke(fn));
		}

		template <class T>
		T* init(tools::string_wrapper&& name)
		{
			auto uptr = utl::make_unique<T>( );
			auto obj  = static_cast<T*>(uptr.get( ));
			page__.emplace<unique_page>(utl::move(uptr));
			name__.init(utl::move(name));

			return obj;
		}

		template <class T>
		T* init(tools::string_wrapper&& name, T* obj)
		{
			page__.emplace<ref_page>(utl::ref(*obj));
			name__.init(utl::move(name));

			return obj;
		}

		template <class T>
		T* init(tools::string_wrapper&& name, const utl::shared_ptr<T>& obj)
		{
			page__.emplace<shared_page>(obj);
			name__.init(utl::move(name));

			return obj.get( );
		}

		template <std::invocable T>
		auto init(tools::string_wrapper&& name, T&& fn)
		{
			return init(utl::move(name), utl::invoke(fn));
		}

		const tools::string_wrapper& name( ) const;
		renderable_object*           page( ) const;

		void render( );

	private:
		using unique_page = utl::unique_ptr<renderable_object>;
		using shared_page = utl::shared_ptr<renderable_object>;
		using ref_page = utl::reference_wrapper<renderable_object>;

		utl::variant<unique_page, shared_page, ref_page> page__;
		tools::string_wrapper_abstract                   name__;
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

		size_t pages_count( ) const;

	protected:
		utl::vector<pages_storage_data> pages_;
	};
}
