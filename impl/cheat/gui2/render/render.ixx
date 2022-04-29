module;

#include <memory>

export module cheat.gui2.render;
export import cheat.gui2.object;

export namespace cheat::gui2
{
	class render : public virtual type_info
	{
	public:
		virtual ~render( );

		virtual bool begin( ) = 0;
		virtual void run(objects_storage<>* const root_storage) = 0;
		virtual bool end( ) = 0;
	};
}
