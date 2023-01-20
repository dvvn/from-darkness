#include <fd/hook_storage.h>

using namespace fd;

void hooks_storage::store(basic_hook* hook)
{
    hooks_.emplace_back(hook);
}

void hooks_storage::store(basic_hook& hook)
{
    hooks_.emplace_back(&hook);
}

bool hooks_storage::enable()
{
    hooks_.shrink_to_fit();
    for (const auto h : hooks_)
    {
        if (!h->active() && !h->enable())
            return false;
    }
    return true;
}

bool hooks_storage::disable()
{
    for (auto itr = hooks_.rbegin(); itr != hooks_.rend(); ++itr)
    {
        auto h = *itr;
        if (h->active() && !h->disable())
            return false;
    }
    return true;
}