#pragma once

#include <boost/hana/tuple.hpp>

namespace fd::gui::detail
{
template <typename... T>
class obj_holder
{
    using storage_type = boost::hana::tuple<T...>;

    storage_type storage_;

  public:
    template <typename... Tfwd>
    obj_holder(Tfwd&&... obj)
        : storage_{std::forward<Tfwd>(obj)...}
    {
    }

    /*template <typename... Tnew>
    objects_holder<T..., std::remove_cvref_t<Tnew>...> add(Tnew&&... new_object) &&
    {
        return boost::hana::unpack([&new_object](T&... obj) {
            return boost::hana::make_tuple(std::move(obj)..., std::forward<Tnew>(new_object)...);
        });
    }

    template <typename... Tnew>
    objects_holder<T..., std::remove_cvref_t<Tnew>...> add(Tnew&&... new_object) const&
    {
        return boost::hana::unpack([&new_object](T const&... obj) {
            return boost::hana::make_tuple(obj..., std::forward<Tnew>(new_object)...);
        });
    }*/

    void operator()() const
    {
        boost::hana::unpack(storage_, [](T const&... obj) {
            (obj(), ...);
        });
    }
};

template <typename... T>
obj_holder(T&&...) -> obj_holder<std::remove_cvref_t<T>...>;
} // namespace fd::gui::detail