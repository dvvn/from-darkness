module;

#include "includes.h"

export module cheat.service:root;
export import :impl;

export namespace cheat
{
	class services_loader final : public static_service<services_loader>
	{
	public:
		struct lazy_reset
		{
			virtual ~lazy_reset( ) = default;
		};

		using reset_object = std::unique_ptr<lazy_reset>;


		~services_loader( ) override;
		services_loader( );

		services_loader(services_loader&& other) = default;
		services_loader& operator=(services_loader&& other) = default;

#ifndef CHEAT_GUI_TEST
		HMODULE my_handle( ) const;
	private:
		using basic_service::load;
	public:
		void load(HMODULE handle);
		std::stop_token load_thread_stop_token( ) const;
#else
		bool load( );
#endif
		void unload( );

		reset_object reset( );

		using executor_shared = std::shared_ptr<executor>;
		executor_shared get_executor(size_t threads_count = std::thread::hardware_concurrency( ));

	protected:
		void load_async( ) noexcept override;
		bool load_impl( ) noexcept override;

	private:
		std::weak_ptr<executor> executor_;
#ifndef CHEAT_GUI_TEST
		HMODULE own_handle_ = nullptr;
		std::jthread load_thread_;
#endif
	};
}
