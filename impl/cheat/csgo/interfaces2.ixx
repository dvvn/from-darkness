module;

export module cheat.csgo.interface_accesser;
import nstd.one_instance;

export namespace cheat::csgo
{
	template<typename Ifc>
	class interface_getter
	{
		Ifc* item_;
	public:

		interface_getter( )
		{
			item_ = this->set( );
		}

		operator Ifc& () const
		{
			return *item_;
		}

		Ifc* set( )const;

	};

	template<typename Ifc>
	using interface_accesser = nstd::one_instance<Ifc, 0, interface_getter<Ifc>>;
}