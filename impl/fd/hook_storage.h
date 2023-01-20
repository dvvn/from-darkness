#pragma once

#include <fd/hook.h>

#include <vector>

namespace fd
{
    class hooks_storage final
    {
        std::vector<basic_hook*> hooks_;

      public:
        void store(basic_hook* hook);
        void store(basic_hook& hook);

        bool enable();
        bool disable();
    };

}