module;

#include "root_includes.h"

export module cheat.root_service;
export import cheat.service;

template<size_t I, class ...T>
decltype(auto) _Get(T&&...args)
{
	return std::get<I>(std::forward_as_tuple(std::forward<T>(args)...));
}

namespace cheat
{
	export class services_loader final : public static_service<services_loader>
	{
	public:
		struct lazy_reset
		{
			virtual ~lazy_reset( ) = default;
		};
		using reset_object = std::unique_ptr<lazy_reset>;
		using promise_type = std::promise<bool>;
		using async_task_type = std::future<bool>;
		using thread_type = std::jthread;

		~services_loader( ) override;
		services_loader( );

		services_loader(services_loader&& other) = default;
		services_loader& operator=(services_loader&& other) = default;

		struct async_detach { };

		template<class ...Args>
		[[nodiscard]] auto start_async(Args&&...args)
		{
			constexpr size_t args_count = sizeof...(args);
			if constexpr (args_count == 0)
			{
				return start_async(std::make_unique<executor>( ));
			}
			else if constexpr (args_count == 1)
			{
				using arg_t = std::remove_cvref_t<Args...>;
				if constexpr (std::same_as<arg_t, async_detach>)
				{
					return start_async(std::make_unique<executor>( ), async_detach( ));
				}
				else
				{
					promise_type prom;
					auto f = prom.get_future( );

					load_thread = thread_type([this, ex = _Get<0>(std::forward<Args>(args)...), prom2 = std::move(prom)]( )mutable
					{
						auto started = this->start(*ex, sync_start( ));
						prom2.set_value(std::move(started));
					});

					return f;
				}
			}
			else
			{
				static_assert(args_count == 2, "Incorrect arguments count!");
				using tag_t = std::remove_cvref_t<decltype(_Get<1>(args...))>;
				static_assert(std::same_as<tag_t, async_detach>, "Incorrect tag type!");

				load_thread = thread_type([this, ex = _Get<0>(std::forward<Args>(args)...)]( )
				{
					[[maybe_unused]]
					const auto started = this->start(*ex, sync_start( ));
					runtime_assert(started == true, "Unable to start the service!");
				});
			}
		}

		template<class ...Args>
		async_task_type start_async(std::shared_ptr<executor>&& ex, Args&&...) = delete;

		void unload( );
		reset_object reset(bool deps_only);

		thread_type load_thread;
		void* module_handle = nullptr;

	protected:
		void construct( ) noexcept override;
		bool load( ) noexcept override;
	};
}
