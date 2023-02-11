#include <fd/hook_storage.h>
#include <fd/views.h>

namespace fd
{
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
    for (const auto h : range_view(hooks_))
    {
        if (h->active())
            continue;
        if (h->enable())
            continue;
        return false;
    }
    return true;
}

bool hooks_storage::disable()
{
    for (const auto h : reverse(hooks_))
    {
        if (!h->active())
            continue;
        if (h->disable())
            continue;
        return false;
    }
    return true;
}
}