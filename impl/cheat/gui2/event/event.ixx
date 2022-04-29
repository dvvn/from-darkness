module;

export module cheat.gui2.event;
export import cheat.gui2.type_info;
export import cheat.gui2.factory;

export namespace cheat::gui2
{
	class event : public virtual type_info, public unique_factory<event>
	{
	public:

	private:
	};
}