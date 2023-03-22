#pragma once

#include <cstdint>

namespace fd::valve
{
class handle;

struct entity_handle
{
    virtual ~entity_handle()               = default;
    virtual void set(handle const &handle) = 0;
    virtual handle const &get() const      = 0;
};

class handle
{
    unsigned long index_;

  public:
    handle();
    handle(handle const &other);
    handle(entity_handle *handle_obj);
    handle(int index, int serial_number);

    // Even if this returns true, Get() still can return return a non-null value.
    // This just tells if the handle has been initted with any values.
    explicit operator bool() const;

    int32_t entry_index() const;
    int32_t serial_number() const;

    int32_t to_int() const;

    // auto operator<=>(const  handle&) const = default;
};
}