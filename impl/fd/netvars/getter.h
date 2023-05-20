#pragma once

#include "core.h"
#include "tag.h"

#include <fd/magic_cast.h>

#include <mutex>
#ifndef _DEBUG
#include <variant>
#endif

namespace fd
{
struct basic_netvar_info
{
    netvar_tag source;
    netvar_tag id;

    consteval basic_netvar_info(netvar_tag source, netvar_tag id)
        : source(source)
        , id(id)
    {
    }
};

#ifdef _DEBUG
using basic_netvar_info_debug = basic_netvar_info;
#else
struct basic_netvar_info_debug
{
    constexpr basic_netvar_info_debug(...)
    {
    }
};
#endif

// #undef _DEBUG

enum class netvar_getter_mode : uint8_t
{
    runtime,
    compile,
};

template <typename T, netvar_getter_mode Mode>
class netvar_getter;

template <typename T>
to<T &> get_netvar(std::in_place_type_t<T>, to<uintptr_t> thisptr, size_t offset)
{
    return thisptr + offset;
}

template <typename T>
to<T *> get_netvar(std::in_place_type_t<T *>, to<uintptr_t> thisptr, size_t offset)
{
    return thisptr + offset;
}

template <typename T>
class netvar_getter<T, netvar_getter_mode::runtime>
{
#ifdef _DEBUG
    static constexpr size_t unset_offset = -1;

    basic_netvar_info info_;
    size_t offset_;
    std::mutex offset_lock_;
#else
    class locked_mutex
    {
        std::mutex mtx_;

      public:
        ~locked_mutex()
        {
            mtx_.unlock();
        }

        locked_mutex()
        {
            mtx_.lock();
        }

        [[nodiscard]]
        auto make_guard()
        {
            return std::lock_guard(mtx_);
        }
    };

    struct ptr_mutex : std::shared_ptr<locked_mutex>
    {
        ptr_mutex()
            : std::shared_ptr<locked_mutex>(std::make_shared<locked_mutex>())
        {
        }
    };

    using data_type = std::variant<size_t, basic_netvar_info, ptr_mutex>;
    data_type data_;
#endif

    void update()
    {
#ifdef _DEBUG
        if (offset_ == unset_offset)
        {
            auto guard = std::lock_guard(offset_lock_);
            if (offset_ == unset_offset)
            {
                offset_ = get_netvar_offset(info_.source, info_.id);
            }
        }
#else
        if (std::holds_alternative<size_t>(data_))
            return;

        struct visitor
        {
            data_type *data;

            void operator()(basic_netvar_info info) const noexcept
            {
                auto mtx    = data->template emplace<ptr_mutex>();
                auto offset = get_netvar_offset(info.source, info.id);
                data->template emplace<size_t>(offset);
            }

            void operator()(ptr_mutex mtx) const noexcept
            {
                auto guard = mtx->make_guard();
            }

            void operator()(size_t val) const
            {
                std::unreachable();
            }
        };

        std::visit(visitor(&data_), data_);
#endif
    }

  public:
    netvar_getter(basic_netvar_info info)
        :
#ifdef _DEBUG
        info_(std::move(info))
        , offset_(unset_offset)
#else
        data_(std::move(info))
#endif
    {
    }

    auto get(void *thisptr)
    {
        update();
#ifndef _DEBUG
        // ReSharper disable once CppInconsistentNaming
        auto offset_ = std::get<size_t>(data_);
#endif
        return get_netvar(std::in_place_type<T>, thisptr, offset_);
    }
};

template <typename T>
class netvar_getter<T, netvar_getter_mode::compile>
{
    [[no_unique_address]] //
    basic_netvar_info_debug info_;
    size_t offset_;

  public:
    consteval netvar_getter(basic_netvar_info_debug info, size_t offset)
        : info_(std::move(info))
        , offset_(offset)
    {
    }

    auto get(void *thisptr) const
    {
        return get_netvar(std::in_place_type<T>, thisptr, offset_);
    }
};

template <typename T>
netvar_getter(std::in_place_type_t<T>, auto) -> netvar_getter<T, netvar_getter_mode::runtime>;

template <typename T>
netvar_getter(std::in_place_type_t<T>, auto, size_t) -> netvar_getter<T, netvar_getter_mode::compile>;
} // namespace fd