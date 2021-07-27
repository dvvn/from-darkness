#pragma once
#include "cheat/core/service.h"

namespace cheat::gui
{
	class renderer final: public service<renderer>
	{
	public:
		~renderer( ) override;
		renderer( );

	protected:
		bool Do_load( ) override;

	public:
		void present(IDirect3DDevice9* d3d_device);
		void reset(IDirect3DDevice9* d3d_device);
	};
}
