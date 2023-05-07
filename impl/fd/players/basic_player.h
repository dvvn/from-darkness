﻿#pragma once

namespace fd
{
class basic_player
{
    void *entity_;

  public:
    basic_player(void *entity);

    void *native() const;

    bool valid() const;
    void destroy();

    bool operator==(basic_player other) const;
};
} // namespace fd