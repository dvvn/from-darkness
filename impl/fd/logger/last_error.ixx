module;

export module fd.system.last_error;
export import fd.format;

struct last_error
{
#ifdef _WIN32
    using size_type = unsigned long;
#else
#pragma error not implemented
#endif

  private:
    size_type id_;

  public:
    last_error();

    operator size_type() const;

    operator fd::string() const;
    operator fd::wstring() const;
};

export namespace fd
{
    using ::last_error;

    template <typename C>
    struct formatter<last_error, C> : formatter<basic_string<C>, C>
    {
        template <class FormatContext>
        auto format(const last_error err, FormatContext& fc) const
        {
            const basic_string<C> str = err;
            return formatter<basic_string<C>, C>::format(str, fc);
        }
    };
} // namespace fd
