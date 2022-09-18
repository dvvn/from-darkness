module;

export module fd.demangle_symbol;
export import fd.string;

export namespace fd
{
    string demangle_symbol(const char* mangled_name);
}
