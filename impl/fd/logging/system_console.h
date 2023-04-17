#pragma once
#include <fd/logging/logger.h>

namespace fd
{
struct system_console : protected virtual log_builder
{
    
    
  protected:
    void write(pointer msg, size_t length) override;
    void write(wpointer msg, size_t length) override;

    void write_before(itr it) const override;
    void write_before(witr it) const override;

    void write_after(itr it) const override;
    void write_after(witr it) const override;
};

} // namespace fd