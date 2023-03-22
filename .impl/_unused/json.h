#pragma once



namespace fd
{
template <template <typename... Args> class ObjectType>
using json_hint = nlohmann::basic_json<ObjectType, std::vector, std::string, bool, intptr_t, uintptr_t, float>;


} // namespace fd