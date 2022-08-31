module;

#include <nlohmann/json.hpp>

export module fd.json;

template <typename T>
concept string_viewable = requires(const T& val)
{
    std::basic_string_view(val);
};

template <typename Key, typename Value, class IgnoredLess = void, class Allocator = std::allocator<std::pair</* const  */ Key, Value>>>
class fake_map : public std::vector<typename Allocator::value_type, Allocator>
{
    using _Base = std::vector<typename Allocator::value_type, Allocator>;

    using _Base::operator[];
    using _Base::at;
    using _Base::emplace_back;
    using _Base::pop_back;
    using _Base::push_back;

    template <typename Key2>
    static decltype(auto) _Correct_key_find(const Key2& key)
    {
        if constexpr (string_viewable<key_type>)
            return std::basic_string_view(key);
        else
            return key;
    }

    template <typename Key2>
    static decltype(auto) _Correct_key_emplace(Key2&& key)
    {
        if constexpr (string_viewable<key_type>)
        {
            using key_t     = std::remove_cvref_t<Key2>;
            using key_raw_t = decltype(key);

            if constexpr (!std::is_class_v<key_t>)
                return std::basic_string_view(key);
            else if constexpr (std::is_rvalue_reference_v<key_raw_t>)
                return std::basic_string(std::move(key));
            else
                return std::basic_string_view(key);
        }
        else
            return std::forward<Key2>(key);
    }

  public:
    using key_type       = Key;
    using mapped_type    = Value;
    using iterator       = typename _Base::iterator;
    using const_iterator = typename _Base::const_iterator;
    using size_type      = typename _Base::size_type;
    using value_type     = typename _Base::value_type;

    using _Base::_Base;

    template <std::equality_comparable_with<Key> Key2>
    iterator find(const Key2& key)
    {
        decltype(auto) key_1 = _Correct_key_find(key);
        return std::find_if(_Base::begin(), _Base::end(), [&](const auto& p) {
            return p.first == key_1;
        });
    }

    template <typename Key2>
    const_iterator find(const Key2& key) const
    {
        return const_cast<fake_map*>(this)->find(key);
    }

    template <typename Key2, typename... Args>
    std::pair<iterator, bool> emplace(Key2&& key, Args&&... args)
    {
        decltype(auto) key_1 = _Correct_key_emplace(std::forward<Key2>(key));
        const auto found     = this->find(key_1);
        if (found != _Base::end())
            return { found, false };
        _Base::emplace_back(std::forward<decltype(key_1)>(key_1), mapped_type(std::forward<Args>(args)...));
        return { std::prev(_Base::end()), true };
    }

    template <typename Key2>
    mapped_type& at(const Key2& key)
    {
        const auto itr = this->find(key);
        if (itr == _Base::end())
            throw std::out_of_range("key not found");
        return itr->second;
    }

    template <typename Key2>
    const mapped_type& at(const Key2& key) const
    {
        return const_cast<fake_map*>(this)->at(key);
    }

    template <typename Key2>
    mapped_type& operator[](Key2&& key)
    {
        static_assert(std::default_initializable<mapped_type>);
        return this->emplace(std::forward<Key2>(key)).first->second;
    }

    template <typename Key2>
    const mapped_type& operator[](const Key2& key) const
    {
        return this->find(key)->second;
    }

    template <typename Key2>
    size_type erase(const Key2& key)
    {
        decltype(auto) key_1 = _Correct_key_find(key);
        const auto old_size  = _Base::size();
        std::remove_if(_Base::begin(), _Base::end(), [&](const auto& p) {
            return p.first == key_1;
        });
        return old_size - _Base::size();
    }

    template <typename Key2>
    size_type count(const Key2& key) const
    {
        decltype(auto) key_1 = _Correct_key_find(key);
        return std::count_if(_Base::begin(), _Base::end(), [&](const auto& p) {
            return p.first == key_1;
        });
    }

    std::pair<iterator, bool> insert(value_type&& value)
    {
        return this->emplace(std::move(value.first), std::move(value.second));
    }

    std::pair<iterator, bool> insert(const value_type& value)
    {
        return this->emplace(value.first, value.second);
    }
};

#if 0
// hack
export namespace nlohmann::detail
{
    namespace detail
    {
        using ::nlohmann::detail::json_sax_dom_callback_parser;
    }
} // namespace nlohmann
#endif

export NLOHMANN_JSON_NAMESPACE_BEGIN

    namespace detail
{
    using ::NLOHMANN_JSON_NAMESPACE::detail::json_sax_dom_callback_parser;
}
NLOHMANN_JSON_NAMESPACE_END

export namespace fd
{
    template <typename... T>
    using adl_serializer = ::nlohmann::adl_serializer<T...>;

    template <template <typename... Args> class ObjectType>
    using _Json = nlohmann::basic_json<ObjectType, std::vector, std::string, bool, intptr_t, uintptr_t, float, std::allocator, adl_serializer>;

    using json          = _Json<std::map>;
    using json_unsorted = _Json<std::map>;
} // namespace fd
