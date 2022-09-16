module;

export module fd.demangle_symbol;
export import fd.string;

fd::string demangle_symbol(const char* mangled_name);

export namespace fd
{
    using ::demangle_symbol;
}
