module;

#include <nstd/runtime_assert_fwd.h>

export module cheat.csgo.structs:BaseHandle;

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

export namespace cheat::csgo
{
	class CBaseHandle;
	class IHandleEntity
	{
	public:
		virtual                    ~IHandleEntity() = default;
		virtual void               SetRefEHandle(const CBaseHandle& handle) = 0;
		virtual const CBaseHandle& GetRefEHandle() const = 0;
	};

	class CBaseHandle
	{
	public:
		CBaseHandle()
		{
			m_Index = INVALID_EHANDLE_INDEX;
		}

		CBaseHandle(const CBaseHandle& other)
		{
			m_Index = other.m_Index;
		}

		CBaseHandle(IHandleEntity* pHandleObj)
		{
			if (!pHandleObj)
			{
				m_Index = INVALID_EHANDLE_INDEX;
			}
			else
			{
				*this = pHandleObj->GetRefEHandle();
			}
		}

		CBaseHandle(int iEntry, int iSerialNumber)
		{
			runtime_assert(iEntry >= 0 && (iEntry & ENT_ENTRY_MASK) == iEntry);
			runtime_assert(iSerialNumber >= 0 && iSerialNumber < (1 << NUM_SERIAL_NUM_BITS));
			m_Index = iEntry | (iSerialNumber << /*NUM_ENT_ENTRY_BITS*/NUM_SERIAL_NUM_SHIFT_BITS);
		}

		// Even if this returns true, Get() still can return return a non-null value.
		// This just tells if the handle has been initted with any values.
		bool IsValid() const
		{
			return m_Index != INVALID_EHANDLE_INDEX;
		}

		int GetEntryIndex() const
		{
			if (!IsValid())
				return NUM_ENT_ENTRIES - 1;
			return m_Index & ENT_ENTRY_MASK;
		}

		int GetSerialNumber() const
		{
			return m_Index >> /*NUM_ENT_ENTRY_BITS*/NUM_SERIAL_NUM_SHIFT_BITS;
		}

		int ToInt() const
		{
			return (int)m_Index;
		}

		//auto operator<=>(const CBaseHandle&) const = default;

		IHandleEntity* Get() const;


	protected:
		// The low NUM_SERIAL_BITS hold the index. If this value is less than MAX_EDICTS, then the entity is networkable.
		// The high NUM_SERIAL_NUM_BITS bits are the serial number.
		unsigned long		 m_Index;
	};
}
