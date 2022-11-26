module;

module fd.netvars.basic_storage;

// bool basic_storage::contains_duplicate(const hashed_string_view name, netvar_table* const from) const
//{
//	const auto begin = this->data( );
//	const auto end = begin + this->size( );
//
//	const auto pos = from ? from : begin;
//	FD_ASSERT(std::distance(begin, pos) >= 0);
//
//	const auto found1 = _Find_name(std::span(pos, end), name);
//	if(found1)
//		return _Find_name(std::span(found1 + 1, end), name);
//	return false;
// }

auto basic_storage::find(const hashed_string_view name) const -> const netvar_table*
{
    return const_cast<basic_storage*>(this)->find(name);
}

auto basic_storage::find(const hashed_string_view name) -> netvar_table*
{
    for (netvar_table& entry : *this)
    {
        if (entry.name() != name)
            continue;
        return std::addressof(entry);
    }
    return nullptr;
}

auto basic_storage::add(netvar_table&& table, const bool skip_find) -> netvar_table*
{
    if (!skip_find)
    {
        const auto existing = find(table.name());
        if (existing)
            return existing;
    }
    this->push_back(std::move(table));
    return nullptr;
}

auto basic_storage::add(hashed_string&& name, const bool skip_find) -> netvar_table*
{
    if (!skip_find)
    {
        const auto existing = find(name);
        if (existing)
            return existing;
    }
    this->emplace_back(std::move(name));
    return nullptr;
}
