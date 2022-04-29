module;

#include <d3d9.h>

export module cheat.gui2.texture.d3d9;
import cheat.gui2.texture;

export namespace cheat::gui2
{
	class texture_d3d9 :public texture
	{
	public:
		~texture_d3d9( ) override;
		texture_d3d9(IDirect3DTexture9* const tex);

		void update_texture( ) noexcept override;

	private:
		IDirect3DTexture9* texture_;
	};
}