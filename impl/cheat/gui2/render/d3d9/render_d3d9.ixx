module;

#include <d3d9.h>

export module cheat.gui2.render.d3d9;
import cheat.gui2.render;

export namespace cheat::gui2
{
	class texture_d3d9 final :public render, public type_info_for<texture_d3d9>
	{
	public:
		using pointer = IDirect3DDevice9* const;

		~texture_d3d9( ) override;
		texture_d3d9(pointer ptr);

		bool begin( ) override;
		void run(objects_storage<>* const root_storage) override;
		bool end( ) override;

	private:
		pointer device_;
	};
}