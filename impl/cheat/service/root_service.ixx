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

		void load_async(std::shared_ptr<executor>&& ex) = delete;
		void load_async(const std::shared_ptr<executor>& ex);
		void load_async(std::unique_ptr<executor>&& ex);
		bool load_sync( );

		void unload( );
		reset_object reset( );

		std::jthread load_thread;
		HMODULE module_handle = nullptr;

	protected:
		void load_async( ) noexcept override;
		bool load_impl( ) noexcept override;
	};
}
