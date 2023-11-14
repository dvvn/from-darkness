#pragma once

namespace fd
{
class native_entity_handle
{
    using size_type = unsigned long;

    size_type value;

  public:
    size_type index() const;
};

}