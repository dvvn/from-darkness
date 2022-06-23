module;

#include <fd/core/assert.h>
#include <fd/core/object.h>

#include <Windows.h>

#include <vector>

module fd.hooks_loader;
import fd.application_info;

[[noreturn]] static DWORD WINAPI _Unload_delayed(LPVOID)
{
    Sleep(1000);
    FreeLibraryAndExitThread(fd::app_info->module_handle, FALSE);
}

static void _Unload_app()
{
    if (fd::app_info->module_handle == GetModuleHandle(nullptr))
        PostQuitMessage(FALSE);
    else
        CreateThread(NULL, 0, _Unload_delayed, nullptr, 0, NULL);
}

class hooks_loader_impl : public basic_hooks_loader
{
    std::vector<hook_base*> hooks_;

  public:
    /* ~hooks_loader_impl() override
    {
        hooks_loader_impl::disable_all();
    } */

    void disable() override
    {
        for (auto h : hooks_)
            h->disable();
    }

    void unload() override
    {
        disable();
        _Unload_app();
    }

  protected:
    bool init(const bool stop_on_error) override
    {
        bool ok = true;

        for (auto h : hooks_)
        {
            if (h->active())
                continue;
            if (!h->initialized())
                h->init();
            if (h->enable())
                continue;
            ok = false;
            if (stop_on_error)
                break;
        }

        return ok;
    }

    void store(hook_base* const hook) override
    {
        for (const auto h : hooks_)
        {
            if (h == hook)
                return;
        }

        hooks_.push_back(hook);
    }
};

FD_OBJECT_BIND_TYPE(hooks_loader, hooks_loader_impl);
