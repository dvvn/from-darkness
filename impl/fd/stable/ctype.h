#pragma once

#define CTYPE_RET_is(_T_) bool
#define CTYPE_RET_to(_T_) _T_

#define CTYPE_ADP(_PREFIX_, _NAME_) adp##_PREFIX_##_NAME_

#define CTYPE_IFC(_PREFIX_, _NAME_)                                                  \
    struct CTYPE_ADP(_PREFIX_, _NAME_)                                               \
    {                                                                                \
        CTYPE_RET_##_PREFIX_(char) operator()(const char chr) const;                 \
        CTYPE_RET_##_PREFIX_(wchar_t) operator()(const wchar_t chr) const;           \
    };                                                                               \
    export namespace fd                                                              \
    {                                                                                \
        constexpr ctype_##_PREFIX_<CTYPE_ADP(_PREFIX_, _NAME_)> _PREFIX_##_##_NAME_; \
    }

#define CTYPE_IMPL(_PREFIX_, _NAME_)                                                                     \
    auto CTYPE_ADP(_PREFIX_, _NAME_)::operator()(const char chr) const->CTYPE_RET_##_PREFIX_(char)       \
    {                                                                                                    \
        const auto result = std::_PREFIX_##_NAME_(static_cast<int>(chr));                                \
        return static_cast<CTYPE_RET_##_PREFIX_(char)>(result);                                          \
    }                                                                                                    \
    auto CTYPE_ADP(_PREFIX_, _NAME_)::operator()(const wchar_t chr) const->CTYPE_RET_##_PREFIX_(wchar_t) \
    {                                                                                                    \
        const auto result = std::_PREFIX_##w##_NAME_(static_cast<wint_t>(chr));                          \
        return static_cast<CTYPE_RET_##_PREFIX_(wchar_t)>(result);                                       \
    }
// template class ctype_##_PREFIX_<CTYPE_ADP(_PREFIX_, _NAME_)>;
