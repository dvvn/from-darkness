#pragma once

#include <fd/netvar_info.h>
#include <fd/utility.h>

#include <memory>
#include <vector>

namespace fd
{
    class netvar_table : public std::vector<std::unique_ptr<basic_netvar_info>>
    {
        string name_;
        bool root_;

      protected:
        void validate_item(const basic_netvar_info* info) const;

      public:
        netvar_table(string&& name, const bool root);

        string_view name() const;
        bool root() const;
        const basic_netvar_info* find(const string_view name) const;

        template <typename... Args>
        const auto* add(Args&&... args)
        {
            auto ptr = make_netvar_info(std::forward<Args>(args)...);
#ifdef _DEBUG
            validate_item(ptr);
#endif
            this->emplace_back(ptr /* , make_deleter(ptr) */);
            return ptr;
        }
    };

    /* class netvar_data_table
    {
    };

    class netvar_table_multi : public netvar_table
    {
        std::variant<std::monostate, netvar_table_multi> inner_;

      public:
        bool have_inner() const;
        netvar_table_multi& inner(string&& name);
        netvar_table_multi& inner();
        const netvar_table_multi& inner() const;
    }; */
} // namespace fd
