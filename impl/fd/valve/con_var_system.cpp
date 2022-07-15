#define FD_CHECK_WHOLE_CVAR_NAME
module;

#include <fd/object.h>
#ifndef FD_CHECK_WHOLE_CVAR_NAME
#include <fd/assert.h>
#endif

//#ifdef FD_CHECK_WHOLE_CVAR_NAME
//#include <sstream>
//#endif
#include <functional>

module fd.valve.con_var;
import fd.rt_modules;
import fd.logger;
import fd.address;

using namespace fd::valve;

FD_OBJECT_IMPL(con_var_system, fd::rt_modules::vstdlib.find_interface<"VEngineCvar">());

using fd::basic_address;

template <typename T>
static void _Set_helper(con_var* ptr, size_t index, T value)
{
    // return dhooks::_Call_function(static_cast<void(con_var::*)(T)>(&con_var::set), ptr, index, value);
    // dhooks::invoke(&con_var::set<T>, index, ptr, value);

    const decltype(&con_var::set<T>) fn = basic_address(ptr).deref<1>()[index];
    std::invoke(fn, ptr, value);
}

template <typename T>
static T _Get_helper(const con_var* ptr, size_t index)
{
    // return dhooks::invoke(&con_var::get<T>, index, ptr);

    const decltype(&con_var::get<T>) fn = basic_address(ptr).deref<1>()[index];
    return std::invoke(fn, ptr);
}

template <>
const char* con_var::get() const
{
    return _Get_helper<const char*>(this, 11);
}

template <>
float con_var::get() const
{
    return _Get_helper<float>(this, 12);
}

template <>
int con_var::get() const
{
    return _Get_helper<int>(this, 13);
}

template <>
bool con_var::get() const
{
    return !!this->get<int>();
}

template <>
void con_var::set(const char* value)
{
    _Set_helper(this, 14, value);
}

template <>
void con_var::set(float value)
{
    _Set_helper(this, 15, value);
}

template <>
void con_var::set(int value)
{
    _Set_helper(this, 16, value);
}

struct ConCommandBaseIterator
{
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
        itr_ = itr_->Next;
        return *this;
    }

    // Postfix increment
    ConCommandBaseIterator operator++(int)
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

static bool _Compare_cvars(const fd::string_view name, const ConCommandBase& other)
{
    if (other.IsCommand())
        return false;
    if (std::memcmp(other.name, name.data(), name.size()) != 0)
        return false;
#ifdef FD_CHECK_WHOLE_CVAR_NAME
    if (other.name[name.size()] != '\0')
        return false;
#endif
    return true;
}

con_var* con_var_system::FindVar(const fd::string_view name) const
{
    const auto comparer                       = std::bind_front(_Compare_cvars, name);
    const ConCommandBaseIterator first_cvar   = basic_address(this).plus(0x30).deref<1>().get<ConCommandBase*>();
    const ConCommandBaseIterator invalid_cvar = nullptr;

    const auto target_cvar = std::find_if(first_cvar, invalid_cvar, comparer);

    if (target_cvar == invalid_cvar)
    {
        std::invoke(fd::logger, "Cvar \"{}\" NOT found", name);
        return nullptr;
    }

#ifdef FD_CHECK_WHOLE_CVAR_NAME
    std::invoke(fd::logger, "Cvar \"{}\" found", name);
#else
    FD_ASSERT(std::find_if(target_cvar + 1, invalid_cvar, comparer) == invalid_cvar, "Found multiple cvars with given name!");
    fd::logger([name] {
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
        const auto known_end = target_cvar->name + name.size();
        // so only look for the zero character
        const auto real_end  = known_end + std::char_traits<char>::length(known_end);
        if (known_end != real_end)
        {
            write_msg(" (full name: ");
            write_braces(fd::string_view(target_cvar->name, real_end));
            write_msg(')');
        }
        write_msg(" found");

        return std::move(msg).str();
    });
#endif

    return static_cast<con_var*>(target_cvar.get());
}
