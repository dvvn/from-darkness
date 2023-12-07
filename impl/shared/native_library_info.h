#pragma once
#include "library_info/impl/function.h"
#include "library_info.h"

namespace fd
{
namespace native
{
class interface_register;
}

class native_library_info : public library_info
{
  protected:
    struct basic_function_getter : library_info::basic_function_getter
    {
        void* create_interface() const
        {
            return find("CreateInterface");
        }
    };

    class basic_interface_getter : public basic_function_getter
    {
        using basic_function_getter::create_interface;

        native::interface_register* root_interface() const;

      protected:
        void* find(string_view name) const;
    };

  public:
    using library_info::library_info;
};

}