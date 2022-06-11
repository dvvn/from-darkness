#define FDS_CHECK_WHOLE_CVAR_NAME
module;

#include <fds/tools/interface.h>

#include <nstd/format.h>
#ifndef FDS_CHECK_WHOLE_CVAR_NAME
#include <fds/core/assert.h>
#endif

//#ifdef FDS_CHECK_WHOLE_CVAR_NAME
//#include <sstream>
//#endif
#include <functional>

module fds.csgo.interfaces.ConVar;
import fds.csgo.modules;
import fds.console;
import nstd.mem.address;

using namespace fds;
using namespace csgo;

FDS_INTERFACE_IMPL(ICVar, csgo_modules::vstdlib.find_interface<"VEngineCvar">());

using nstd::mem::basic_address;

template <typename T>
static void _Set_helper(ConVar* ptr, size_t index, T value)
{
    // return dhooks::_Call_function(static_cast<void(ConVar::*)(T)>(&ConVar::set), ptr, index, value);
    // dhooks::invoke(&ConVar::set<T>, index, ptr, value);

    const decltype(&ConVar::set<T>) fn = basic_address(ptr).deref<1>()[index];
    std::invoke(fn, ptr, value);
}

template <typename T>
static T _Get_helper(const ConVar* ptr, size_t index)
{
    // return dhooks::invoke(&ConVar::get<T>, index, ptr);

    const decltype(&ConVar::get<T>) fn = basic_address(ptr).deref<1>()[index];
    return std::invoke(fn, ptr);
}

template <>
const char* ConVar::get() const
{
    return _Get_helper<const char*>(this, 11);
}

template <>
float ConVar::get() const
{
    return _Get_helper<float>(this, 12);
}

template <>
int ConVar::get() const
{
    return _Get_helper<int>(this, 13);
}

template <>
bool ConVar::get() const
{
    return !!this->get<int>();
}

template <>
void ConVar::set(const char* value)
{
    _Set_helper(this, 14, value);
}

template <>
void ConVar::set(float value)
{
    _Set_helper(this, 15, value);
}

template <>
void ConVar::set(int value)
{
    _Set_helper(this, 16, value);
}

class ConCommandBaseIterator
{
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = ConCommandBase;
    using pointer           = value_type*;
    using reference         = value_type&;

    ConCommandBaseIterator(pointer ptr)
        : itr_(ptr)
    {
    }

    pointer get() const
    {
        return itr_;
    }

    pointer operator->() const
    {
        return itr_;
    }

    reference operator*() const
    {
        return *itr_;
    }

    // Prefix increment
    ConCommandBaseIterator& operator++()
    {
        itr_ = itr_->m_pNext;
        return *this;
    }

    // Postfix increment
    ConCommandBaseIterator operator++(difference_type)
    {
        ConCommandBaseIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    bool operator==(const ConCommandBaseIterator& other) const
    {
        return itr_ == other.itr_;
    }

  private:
    pointer itr_;
};

static bool _Compare_cvars(const std::string_view name, const ConCommandBase& other)
{
    if (other.IsCommand())
        return false;
    if (std::memcmp(other.m_pszName, name.data(), name.size()) != 0)
        return false;
#ifdef FDS_CHECK_WHOLE_CVAR_NAME
    if (other.m_pszName[name.size()] != '\0')
        return false;
#endif
    return true;
}

ConVar* ICVar::FindVar(const std::string_view name) const
{
    const auto comparer                       = std::bind_front(_Compare_cvars, name);
    const ConCommandBaseIterator first_cvar   = basic_address(this).plus(0x30).deref<1>().get<ConCommandBase*>();
    const ConCommandBaseIterator invalid_cvar = nullptr;

    const auto target_cvar = std::find_if(first_cvar, invalid_cvar, comparer);

    if (target_cvar == invalid_cvar)
    {
        console::log("Cvar \"{}\" NOT found", name);
        return nullptr;
    }

#ifdef FDS_CHECK_WHOLE_CVAR_NAME
    console::log("Cvar \"{}\" found", name);
#else
    FDS_ASSERT(std::find_if(target_cvar + 1, invalid_cvar, comparer) == invalid_cvar, "Found multiple cvars with given name!");
    console::log([name] {
        std::ostringstream msg;

        const auto write_msg = [&]<typename T>(T obj) {
            msg << obj;
        };
        const auto write_braces = [&]<typename T>(T obj) noexcept {
            msg << '"' << obj << '"';
        };

        write_msg("Cvar ");
        write_braces(name);

        // we already know how long a string can be
        const auto known_end = target_cvar->m_pszName + name.size();
        // so only look for the zero character
        const auto real_end = known_end + std::char_traits<char>::length(known_end);
        if (known_end != real_end)
        {
            write_msg(" (full name: ");
            write_braces(std::string_view(target_cvar->m_pszName, real_end));
            write_msg(')');
        }
        write_msg(" found");

        return std::move(msg).str();
    });
#endif

    return static_cast<ConVar*>(target_cvar.get());
}
