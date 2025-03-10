#ifndef VI_LAYER_GUI_CONFIG_HPP
#define VI_LAYER_GUI_CONFIG_HPP
#ifdef VI_RMLUI
#include <vitex/core.h>
#include <utility>
#include <vector>
#include <string>
#include <stack>
#include <list>
#include <functional>
#include <queue>
#include <array>
#include <unordered_map>
#include <memory>
#include <RmlUi/Core/Containers/itlib/flat_map.hpp>
#include <RmlUi/Core/Containers/itlib/flat_set.hpp>
#include <RmlUi/Core/Containers/robin_hood.h>
#define RMLUI_MATRIX4_TYPE ColumnMajorMatrix4f
#define RMLUI_RELEASER_FINAL final

namespace Rml
{
	template <typename T, typename Enable = void>
	struct MixedHasher
	{
		inline size_t operator()(const T& Value) const
		{
			auto Result = vitex::core::key_hasher<T>()(Value);
			return robin_hood::hash_int(static_cast<robin_hood::detail::SizeT>(Result));
		}
	};

	template <typename T>
	using Vector = vitex::core::vector<T>;
	template <typename T, size_t N = 1>
	using Array = std::array<T, N>;
	template <typename T>
	using Stack = std::stack<T>;
	template <typename T>
	using List = vitex::core::linked_list<T>;
	template <typename T>
	using Queue = vitex::core::single_queue<T>;
	template <typename T1, typename T2>
	using Pair = std::pair<T1, T2>;
	template <typename Key, typename Value>
	using StableMap = vitex::core::ordered_map<Key, Value>;
	template <typename Key, typename Value>
	using UnorderedMultimap = vitex::core::unordered_multi_map<Key, Value>;
	template <typename Key, typename Value>
	using UnorderedMap = robin_hood::unordered_flat_map<Key, Value, MixedHasher<Key>, vitex::core::equal_to<Key>>;
	template <typename Key, typename Value>
	using SmallUnorderedMap = itlib::flat_map<Key, Value>;
	template <typename T>
	using UnorderedSet = robin_hood::unordered_flat_set<T, MixedHasher<T>, vitex::core::equal_to<T>>;
	template <typename T>
	using SmallUnorderedSet = itlib::flat_set<T>;
	template <typename T>
	using SmallOrderedSet = itlib::flat_set<T>;
	template<typename Iterator>
	inline std::move_iterator<Iterator> MakeMoveIterator(Iterator It)
	{
		return std::make_move_iterator(It);
	}
	template <typename T>
	using Hash = typename vitex::core::key_hasher<T>;
	template <typename T>
	using Function = std::function<T>;
	using String = vitex::core::string;
	using StringList = Vector<String>;
	template <typename T>
	using UniquePtr = std::unique_ptr<T>;
	template <typename T>
	class Releaser;
	template <typename T>
	using UniqueReleaserPtr = std::unique_ptr<T, Releaser<T>>;
	template <typename T>
	using SharedPtr = std::shared_ptr<T>;
	template <typename T>
	using WeakPtr = std::weak_ptr<T>;
	template <typename T, typename... Args>
	inline SharedPtr<T> MakeShared(Args&&... Values)
	{
		return std::make_shared<T, Args...>(std::forward<Args>(Values)...);
	}
	template <typename T, typename... Args>
	inline UniquePtr<T> MakeUnique(Args&&... Values)
	{
		return std::make_unique<T, Args...>(std::forward<Args>(Values)...);
	}
}
#endif
#endif