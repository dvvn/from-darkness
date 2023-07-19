#pragma once


namespace fd
{
struct basic_search_stop_token
{
  protected:
    ~basic_search_stop_token() = default;

  public:
    virtual bool operator()(void *found) const = 0;
};
} // namespace fd
