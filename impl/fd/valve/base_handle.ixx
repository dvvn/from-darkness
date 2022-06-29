module;

#include <cstdint>

export module fd.valve.base_handle;

class base_handle;

class handle_entity
{
  public:
    virtual ~handle_entity()                              = default;
    virtual void SetRefEHandle(const base_handle& handle) = 0;
    virtual const base_handle& GetRefEHandle() const      = 0;
};

class base_handle
{
    // The low NUM_SERIAL_BITS hold the index. If this value is less than MAX_EDICTS, then the entity is networkable.
    // The high NUM_SERIAL_NUM_BITS bits are the serial number.
    unsigned long m_Index;

  public:
    base_handle();
    base_handle(const base_handle& other);
    base_handle(handle_entity* pHandleObj);
    base_handle(int iEntry, int iSerialNumber);

    // Even if this returns true, Get() still can return return a non-null value.
    // This just tells if the handle has been initted with any values.
    bool IsValid() const;

    int32_t GetEntryIndex() const;
    int32_t GetSerialNumber() const;

    int32_t ToInt() const;

    // auto operator<=>(const  base_handle&) const = default;

    handle_entity* Get() const;
};

export namespace fd::valve
{
    using ::base_handle;
    using ::handle_entity;
} // namespace fd::valve
