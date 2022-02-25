module;

#include <cstdint>

export module cheat.csgo.interfaces.BaseHandle;

export namespace cheat::csgo
{
	class CBaseHandle;
	class IHandleEntity
	{
	public:
		virtual                    ~IHandleEntity( ) = default;
		virtual void               SetRefEHandle(const CBaseHandle& handle) = 0;
		virtual const CBaseHandle& GetRefEHandle( ) const = 0;
	};

	class CBaseHandle
	{
	public:
		CBaseHandle( );
		CBaseHandle(const CBaseHandle& other);
		CBaseHandle(IHandleEntity* pHandleObj);
		CBaseHandle(int iEntry, int iSerialNumber);

		// Even if this returns true, Get() still can return return a non-null value.
		// This just tells if the handle has been initted with any values.
		bool IsValid( ) const;

		int32_t GetEntryIndex( ) const;
		int32_t GetSerialNumber( ) const;

		int32_t ToInt( ) const;

		//auto operator<=>(const CBaseHandle&) const = default;
				
		IHandleEntity* Get( ) const;

	protected:
		// The low NUM_SERIAL_BITS hold the index. If this value is less than MAX_EDICTS, then the entity is networkable.
		// The high NUM_SERIAL_NUM_BITS bits are the serial number.
		unsigned long		 m_Index;
	};
}
