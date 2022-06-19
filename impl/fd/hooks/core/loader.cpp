module;

#include <fd/core/assert.h>
#include <fd/core/object.h>

#include <Windows.h>

#include <vector>

module fd.hooks_loader;
import fd.application_info;

[[noreturn]] static DWORD WINAPI unload_delayed(LPVOID)
{
    Sleep(1000);
    FreeLibraryAndExitThread(fd::app_info->module_handle, FALSE);
}

static void unload_app()
{
    if (fd::app_info->module_handle == GetModuleHandle(nullptr))
        PostQuitMessage(FALSE);
    else
        CreateThread(NULL, 0, unload_delayed, nullptr, 0, NULL);
}

bool basic_hooks_loader::load_all()
{
    return load<FD_HOOK_IDS>();
}

class hooks_loader_impl : public basic_hooks_loader
{
    std::vector<hook_base*> hooks_;

  public:
    /* ~hooks_loader_impl() override
    {
        hooks_loader_impl::disable_all();
    } */

    void disable_all() override
    {
        for (auto h : hooks_)
            h->disable();
    }

    void unload() override
    {
        disable_all();
        unload_app();
    }

  protected:
    void load() override
    {
        for (auto h : hooks_)
        {
            if (h->active())
                return;
            if (!h->initialized())
                h->init();
            if (!h->enable())
                throw;
        }
    }

    void store(hook_base* const hook) override
    {
        for (auto h : hooks_)
        {
            if (h == hook)
                return;
        }

        hooks_.push_back(hook);
    }
};

FD_OBJECT_BIND_TYPE(hooks_loader, hooks_loader_impl);
