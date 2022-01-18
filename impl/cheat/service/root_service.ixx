module;

#include "includes.h"

export module cheat.service:root;
export import :impl;

namespace cheat
{
	export class services_loader final : public static_service<services_loader>
	{
		using static_service::load;

		bool load_impl(executor& ex);

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

		template<class T>
		void load_async(T&& ex)
		{
			load_thread = std::jthread(
				[ex2 = std::forward<T>(ex), this]
				{
					if (this->load_impl(*ex2))
						return;
					[[maybe_unused]]
					const auto delayed = this->reset( );
					using namespace std::chrono_literals;
					std::this_thread::sleep_for(200ms);
					::FreeLibrary(this->module_handle);
				});
		}
		template<>
		void load_async(std::shared_ptr<executor>&& ex) = delete;
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
