#pragma once
#include "cheat/core/service.h"

namespace cheat::gui
{
	class renderer final: public service_shared<renderer, service_mode::async>
	{
	public:
		~renderer( ) override;
		renderer( );

	protected:
		void Load( ) override;

	public:
		void present(IDirect3DDevice9* d3d_device);
		void reset(IDirect3DDevice9* d3d_device);

	private:
		bool skip_first_tick__ = true;
	};
}
