#include "tool/string_view.h"
#include "valve_library_info.h"

#include <algorithm>
#include <cassert>
#include <cctype>

namespace fd
{
class valve_interface_holder
{
    using pointer = valve_interface_holder const *;

    void *(*create_)();
    char const *name_;
    pointer next_;

  public:
    valve_interface_holder() = delete;

    bool compare(string_view other_name) const
    {
        return memcmp(name_, other_name.data(), other_name.length());
    }

    void *get() const
    {
        return create_();
    }

    struct find_result
    {
        pointer interface;
        bool fully_compared;

        find_result(pointer interface, bool fully_compared = true)
            : interface(interface)
            , fully_compared(fully_compared)
        {
        }

        // ReSharper disable once CppPossiblyUninitializedMember
        find_result(nullptr_t)
            : interface(nullptr)
        {
        }
    };

    find_result find(string_view target_name) const
    {
        for (auto i = this; i != nullptr; i = i->next_)
        {
            auto name_back = i->name_ + target_name.length();

            auto all_digits_at_end = [=] {
                for (auto c = name_back + 1; *c != '\0'; ++c)
                {
                    if (!isdigit(*c))
                        return false;
                }
                return true;
            };

            if (*name_back == '\0')
            {
                if (i->compare(target_name) == 0)
                    return i;
            }
            else if (isdigit(*name_back))
            {
                if (i->compare(target_name) == 0 && all_digits_at_end())
                    return {i, false};
            }
        }

        return nullptr;
    }

    pointer next() const
    {
        return next_;
    }
};

static valve_interface_holder *root_interface(void *create_fn)
{
    auto relative_fn  = reinterpret_cast<uintptr_t>(create_fn) + 0x5;
    auto displacement = *reinterpret_cast<int32_t *>(relative_fn);
    auto jmp          = relative_fn + sizeof(int32_t) + displacement;

    return **reinterpret_cast<valve_interface_holder ***>(jmp + 0x6);
}

void *valve_library::interface(string_view name) const
{
    auto [interface, fully_compared] = root_interface(function("CreateInterface"))->find(name);

#ifdef _DEBUG
    if (!fully_compared)
    {
        auto next_interface = interface->next();
        if (next_interface)
        {
            auto [duplicate, _] = next_interface->find(name);
            assert(duplicate == nullptr);
        }
    }
#endif

    return interface->get();
}
} // namespace fd