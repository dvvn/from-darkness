#pragma once
#include <cstdint>

namespace fd::valve
{
struct client_class
{
    void *create;
    void *create_event;
    char const *name;
    void *recv_table;
    client_class *next;
    uint32_t id;
};

class client_class_range
{
    void *interface_;

  public:
    class iterator
    {
        client_class *class_;

      public:
        iterator(client_class *class_)
            : class_(class_)
        {
        }

        iterator &operator++()
        {
            class_ = class_->next;
            return *this;
        }

        bool operator==(iterator other) const
        {
            return class_ == other.class_;
        }

        client_class &operator*() const
        {
            return *class_;
        }

        client_class *operator->() const
        {
            return class_;
        }
    };

    client_class_range(void *client_interface)
        : interface_(client_interface)
    {
    }

    iterator begin() const;
    iterator end() const;
};

}