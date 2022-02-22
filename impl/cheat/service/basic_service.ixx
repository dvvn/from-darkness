module;

#include "basic_includes.h"

#include <nstd/type_traits.h>

export module cheat.service:basic;

export namespace cheat
{
	class __declspec(novtable) basic_service
	{
	public:
		using executor = std::shared_ptr<cppcoro::static_thread_pool>;
		using task_type = cppcoro::shared_task<bool>;
		//using mutex_type = cppcoro::async_mutex;
		using value_type = std::shared_ptr<basic_service>;

		enum class state_type :uint8_t
		{
			idle
			, started
			, loaded
			, loaded_error
		};

		using callback_type = std::shared_ptr<std::function<void(basic_service*, state_type)>>;

		using deps_storage_base = std::vector<value_type>;
		class deps_storage : protected deps_storage_base
		{
		public:

			deps_storage( ) = default;

			deps_storage(const deps_storage&) = delete;
			deps_storage& operator=(const deps_storage&) = delete;

			deps_storage(deps_storage&& other) = default;
			deps_storage& operator=(deps_storage&& other) = default;

			size_t add(value_type&& val);
			size_t add(const value_type& val);

			template<typename T>
			size_t add( )
			{
				return add(std::make_shared<T>( ));
			}
			using deps_storage_base::begin;
			using deps_storage_base::end;
			using deps_storage_base::empty;
			using deps_storage_base::size;
			using deps_storage_base::operator [];

			deps_storage_base::iterator find(const std::type_info& type);
			deps_storage_base::const_iterator find(const std::type_info& type)const;
			bool contains(const std::type_info& type)const;
		};

		basic_service( );
		virtual ~basic_service( );

		basic_service(const basic_service& other) = delete;
		basic_service(basic_service&& other) noexcept;
		basic_service& operator=(const basic_service& other) = delete;
		basic_service& operator=(basic_service&& other) noexcept;

		virtual std::string_view name( ) const = 0;
		virtual const std::type_info& type( ) const = 0;

	private:
		task_type load_deps(const executor ex, const callback_type callback)noexcept;
		task_type load_this(const executor ex, const callback_type callback)noexcept;
		task_type start_impl(const executor ex, const callback_type callback) noexcept;

	public:
		void start(const executor ex, const callback_type callback) noexcept;
		void start(const executor ex) noexcept;
		void start( ) noexcept;

	protected:
		//delayed constructor
		virtual void construct( )noexcept = 0;
		virtual bool load( ) noexcept
		{
			return true;
		}

	public:
		std::atomic<state_type> state = state_type::idle;
		task_type start_result;
		deps_storage load_before;
	};
}


