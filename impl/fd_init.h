#pragma once

#include <fd/object.h>
#include <fd/utility.h>

import fd.logger;
import fd.system.console;
import fd.assert;
import fd.async;
import fd.hooks_loader;
import fd.rt_modules;

namespace fd
{
    static bool fd_init_core()
    {
#if defined(_DEBUG) || defined(FD_GUI_TEST)
        logger->append([](const auto str) {
            invoke(system_console_writer, str);
        });
#endif
#ifdef _DEBUG
        assert_handler->push_back([](const assert_data& data) {
            invoke(system_console_writer, data.build_message());
        });
#endif
        return true;
    }

    static void fd_destroy()
    {
        FD_OBJECT_GET(async).destroy();
        FD_OBJECT_GET(hooks_loader).destroy();
    }

    template <class T, class L>
    static void _Store_hook(L& loader)
    {
        const auto ptr      = &FD_OBJECT_GET(T*);
        // UNSAFE
        //  ok in debug mode
        // ??? in release mode
        const auto base_ptr = reinterpret_cast<hook_base*>(ptr);
        loader.store(base_ptr);
    }
} // namespace fd

#define _HOOK_NAME(_N_) hk_##_N_

#define _DECLARE_HOOK(_NAME_) struct _HOOK_NAME(_NAME_);
#define _STORE_HOOK(_NAME_)   _Store_hook<_HOOK_NAME(_NAME_)>(loader);

#define PREPARE_HOOKS(_FN_NAME_, ...)                   \
    FOR_EACH(_DECLARE_HOOK, __VA_ARGS__);               \
    template <class L>                                  \
    static bool _FN_NAME_(L& loader)                    \
    {                                                   \
        FOR_EACH(_STORE_HOOK, __VA_ARGS__);             \
        return loader.enable();                         \
    }                                                   \
    static bool _FN_NAME_()                             \
    {                                                   \
        return _FN_NAME_(*FD_OBJECT_GET(hooks_loader)); \
    }
