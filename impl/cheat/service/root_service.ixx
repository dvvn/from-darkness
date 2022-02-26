module;

#include "basic_includes.h"

export module cheat.root_service;
export import cheat.service;

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

		~services_loader( ) override;
		services_loader( );

		services_loader(services_loader&& other) = default;
		services_loader& operator=(services_loader&& other) = default;

		void unload( );
		reset_object reset(bool deps_only);

		void* module_handle = nullptr;

	protected:
		void construct( ) noexcept override;
		bool load( ) noexcept override;
	};
}
