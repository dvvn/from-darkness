#pragma once

#define CTYPE_RET_is(_T_) bool
#define CTYPE_RET_to(_T_) _T_

#define CTYPE_ADP(_PREFIX_, _NAME_)         adp##_PREFIX_##_NAME_
#define CTYPE_ADP_CT(_PREFIX_, _NAME_)      adp##_PREFIX_##_NAME_##_ct
#define CTYPE_ADP_CT_IMPL(_PREFIX_, _NAME_) adp##_PREFIX_##_NAME_##_ct_impl

#define CTYPE_IFC(_PREFIX_, _NAME_, ...)                                                                             \
    struct CTYPE_ADP(_PREFIX_, _NAME_)                                                                               \
    {                                                                                                                \
        CTYPE_RET_##_PREFIX_(char) operator()(const char chr) const;                                                 \
        CTYPE_RET_##_PREFIX_(wchar_t) operator()(const wchar_t wchr) const;                                          \
    };                                                                                                               \
    template <typename T>                                                                                            \
    constexpr T CTYPE_ADP_CT_IMPL(_PREFIX_, _NAME_)(const T chr);                                                    \
    struct CTYPE_ADP_CT(_PREFIX_, _NAME_)                                                                            \
    {                                                                                                                \
        constexpr CTYPE_RET_##_PREFIX_(char) operator()(const char chr) const                                        \
        {                                                                                                            \
            return CTYPE_ADP_CT_IMPL(_PREFIX_, _NAME_)(chr);                                                         \
        }                                                                                                            \
        constexpr CTYPE_RET_##_PREFIX_(wchar_t) operator()(const wchar_t wchr) const                                 \
        {                                                                                                            \
            return CTYPE_ADP_CT_IMPL(_PREFIX_, _NAME_)(wchr);                                                        \
        }                                                                                                            \
    };                                                                                                               \
    export namespace fd                                                                                              \
    {                                                                                                                \
        constexpr ctype_##_PREFIX_<CTYPE_ADP(_PREFIX_, _NAME_), CTYPE_ADP_CT(_PREFIX_, _NAME_)> _PREFIX_##_##_NAME_; \
    }                                                                                                                \
    template <typename T>                                                                                            \
    constexpr T CTYPE_ADP_CT_IMPL(_PREFIX_, _NAME_)(const T chr)

#define CTYPE_IMPL(_PREFIX_, _NAME_)                                                                      \
    auto CTYPE_ADP(_PREFIX_, _NAME_)::operator()(const char chr) const->CTYPE_RET_##_PREFIX_(char)        \
    {                                                                                                     \
        const auto result = std::_PREFIX_##_NAME_(static_cast<int>(chr));                                 \
        return static_cast<CTYPE_RET_##_PREFIX_(char)>(result);                                           \
    }                                                                                                     \
    auto CTYPE_ADP(_PREFIX_, _NAME_)::operator()(const wchar_t wchr) const->CTYPE_RET_##_PREFIX_(wchar_t) \
    {                                                                                                     \
        const auto result = std::_PREFIX_##w##_NAME_(static_cast<wint_t>(wchr));                          \
        return static_cast<CTYPE_RET_##_PREFIX_(wchar_t)>(result);                                        \
    }
