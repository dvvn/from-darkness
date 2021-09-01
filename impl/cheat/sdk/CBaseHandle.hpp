#pragma once

#include "IHandleEntity.hpp"


namespace cheat::csgo
{
	// How many bits to use to encode an edict.
	constexpr auto MAX_EDICT_BITS = 11; // # of bits needed to represent max edicts
	// Max # of edicts in a level
	constexpr auto MAX_EDICTS = 1 << MAX_EDICT_BITS;

	// Used for networking ehandles.
	constexpr auto NUM_ENT_ENTRY_BITS = MAX_EDICT_BITS + 2;
	constexpr auto NUM_ENT_ENTRIES = 1 << NUM_ENT_ENTRY_BITS;
	constexpr auto INVALID_EHANDLE_INDEX = 0xFFFFFFFF;

	constexpr auto NUM_SERIAL_NUM_BITS = 16/*32 - NUM_ENT_ENTRY_BITS*/;
	constexpr auto NUM_SERIAL_NUM_SHIFT_BITS = 32 - NUM_SERIAL_NUM_BITS;
	constexpr auto ENT_ENTRY_MASK = (1 << NUM_SERIAL_NUM_BITS) - 1;

	class IHandleEntity;

	class CBaseHandle
	{
	public:
		CBaseHandle( );
		CBaseHandle(const CBaseHandle& other);
		/*explicit*/
		CBaseHandle(IHandleEntity* pHandleObj);
		CBaseHandle(int iEntry, int iSerialNumber);

		// Even if this returns true, Get() still can return return a non-null value.
		// This just tells if the handle has been initted with any values.
		bool IsValid( ) const;

		int GetEntryIndex( ) const;
		int GetSerialNumber( ) const;

		int ToInt( ) const;
		auto operator<=>(const CBaseHandle&) const;

		// Use this to dereference the handle.
		// Note: this is implemented in game code (ehandle.h)
		IHandleEntity* Get( ) const;

	protected:
		// The low NUM_SERIAL_BITS hold the index. If this value is less than MAX_EDICTS, then the entity is networkable.
		// The high NUM_SERIAL_NUM_BITS bits are the serial number.
		unsigned long		 m_Index;
	};
}
