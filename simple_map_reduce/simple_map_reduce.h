#pragma once

#include <sstream>
#include <list>
#include <functional>
#include <vector>
#include <map>

#define _ret(expr) ([]() constexpr {return (expr);}) 
#define λ(expr) ([](auto _) constexpr {return (expr);})
#define λ2(expr) ([](auto _1, auto _2) constexpr {return (expr);})
#define λ3(expr) ([](auto _1, auto _2, auto _3) constexpr {return (expr);})
#define λ4(expr) ([](auto _1, auto _2, auto _3, auto _4) constexpr {return (expr);})
#define λn(expr) ([](auto... _) constexpr {return (expr);})

namespace smr
{

	using namespace std;

	const auto identity = λ(_);

	// ___ COMPILETIME FKT MAPREDUCE ___

	template <typename Foldable, typename Res, Res(*fn)(Res, typename Foldable::value_type)>
	__forceinline Res foldl(const Res& init, const Foldable& vec)
	{
		Res res = init;
		for (auto&& e : vec)
			res = fn(res, e);
		return res;
	}

	template <template<typename, typename...> typename ForwardIterable,
		typename In, bool(*predicate)(In)>
		__forceinline ForwardIterable<In> filter(const ForwardIterable<In>& vec)
	{
		ForwardIterable<In> res;
		for (auto&& e : vec) if (predicate(e))
			res.push_back(e);
		return res;
	}

	/** equal to first filtering by predicate, then mapping with fn, but in one iteration. */
	template <template<typename, typename...> typename ForwardIterable,
		typename In, typename Out, Out(*fn)(In), bool(*predicate)(In)>
		__forceinline ForwardIterable<Out> collect(const ForwardIterable<In>& vec)
	{
		ForwardIterable<Out> res;
		for (auto&& e : vec) if (predicate(e))
			res.push_back(fn(e));
		return res;
	}

	template <template<typename, typename...> typename ForwardIterable,
		typename In, typename Out, Out(*fn)(In)>
		__forceinline ForwardIterable<Out> fmap(const ForwardIterable<In>& vec)
	{
		ForwardIterable<Out> res;
		for (auto&& e : vec)
			res.push_back(fn(e));
		return res;
	}

	template <template<typename, typename...> typename ForwardIterable,
		typename Inner, typename In, Inner(*fn)(In)>
		__forceinline auto flatmap(const ForwardIterable<In>& vec)
	{
		using Out = std::decay_t<typename Inner::value_type>;
		ForwardIterable<Out> res;
		for (auto&& e : vec)
			for (auto&& e2 : fn(e))
				res.push_back(e2);
		return res;
	}

	// FIXME: these somehow don't work, 'constant compiletime expression expected'

	//template <template<typename, typename...> typename ForwardIterable, 
	//	typename Out, Out(*generator)(size_t)>
	//__forceinline ForwardIterable<Out> tabulate(const size_t n)
	//{
	//	ForwardIterable<Out> res;
	//	for (size_t i = 0; i < n; ++i)
	//		res.push_back(generator(i));
	//	return res;
	//}
	//
	//template <template<typename, typename...> typename ForwardIterable, 
	//	typename Out, Out(*generator)()>
	//__forceinline ForwardIterable<Out> fill(const size_t n)
	//{
	//	ForwardIterable<Out> res;
	//	for (size_t i = 0; i < n; ++i)
	//		res.push_back(generator());
	//	return res;
	//}

	/** Returns a map with vec's values as keys
	*  and the results of fn applied to each key as values */
	template <template<typename, typename...> typename Traversable,
		typename In, typename Out, typename Out(*fn)(In),
		template <typename, typename, typename...> typename MapT = std::map>
	__forceinline MapT<In, Out> to_map(const Traversable<In>& vec)
	{
		MapT<In, Out> res{};
		for (auto&& e : vec)
			res.insert({ e, fn(e) });
		return res;
	}

	/** Returns a map from the result of fn to a traversable of all elements,
	*  which when put through fn are equal to the key */
	template <template <typename, typename...> typename Traversable,
		typename In, typename Key, typename Key(*fn)(In),
		template <typename, typename, typename...> typename MapT = std::map>
	MapT<Key, Traversable<In>> group_by(const Traversable<In>& vec)
	{
		MapT<Key, Traversable<In>> res{};
		for (auto&& e : vec)
		{
			auto key = fn(e);
			auto search = res.find(key);
			if (search != res.end())
				res[key].push_back(e);
			else res[key] = Traversable<In>{ e };
		};
		return res;
	}

	template <template <typename, typename, typename...> typename MapT = map,
		typename Key, typename In, typename Out, typename Out(*fn)(In)>
		__forceinline constexpr MapT<Key, Out> fmap_values(const MapT<Key, In> map)
	{
		MapT<Key, Out> res;
		for (auto kv : map)
			res[kv.first] = fn(kv.second);
		return res;
	}


	// ___ RUNTIME MAPREDUCE ___


	template <typename ForwardIterable, typename ValueMapper>
	string show(const ForwardIterable& vec, const ValueMapper& fn) {
		// has to look like this so that it works for lists as well
		if (vec.empty()) return "[]";
		ostringstream oss;
		bool first = true;
		for (auto el : vec) {
			if (!first) oss << ", ";
			oss << fn(el);
			first = false;
		}
		return "[" + oss.str() + "]";
	}

	template <typename ForwardIterable>
	__forceinline string show(const ForwardIterable& vec) {
		return show(vec, identity);
	}

	template <typename Writable>
	inline string to_str(const Writable& wr)
	{
		ostringstream oss;
		oss << wr;
		return oss.str();
	}

	template <typename Res, typename Foldable, typename Fn>
	inline auto foldl(const Res& init, const Foldable& vec, const Fn& fn)
	{
		Res res = init;
		for (auto&& e : vec)
			res = fn(res, e);
		return res;
	}

	template<typename MonoidFoldable>
	inline auto sum(const MonoidFoldable& vec)
	{
		using Out = std::decay_t<decltype(*vec.begin())>;
		Out res{};
		for (auto&& e : vec)
			res = res + e;
		return res;
	}

	template <typename ForwardIterable, typename Predicate>
	inline ForwardIterable filter(const ForwardIterable& vec, const Predicate& predicate)
	{
		ForwardIterable res;
		for (auto&& e : vec) if (predicate(e))
			res.push_back(e);
		return res;
	}

	/** equal to first filtering by predicate, then mapping with fn, but in one iteration. */
	template <template<typename, typename...> typename ForwardIterable, typename In, typename Fn, typename Predicate>
	inline auto collect(const ForwardIterable<In>& vec, const Fn& fn, const Predicate& predicate)
	{
		using Out = std::decay_t<decltype((*vec.begin()))>;
		ForwardIterable<Out> res;
		for (auto&& e : vec) if (predicate(e))
			res.push_back(fn(e));
		return res;
	}

	template <template<typename, typename...> typename ForwardIterable, typename In, typename Fn>
	inline auto fmap(const ForwardIterable<In>& vec, const Fn& fn)
	{
		using Out = std::decay_t<decltype(fn(*vec.begin()))>;
		ForwardIterable<Out> res;
		for (auto&& e : vec)
			res.push_back(fn(e));
		return res;
	}

	template <template<typename, typename...> typename Outer, typename Inner>
	inline auto flatten(const Outer<Inner>& vec)
	{
		using Out = std::decay_t<decltype(*(*vec.begin()).begin())>;
		Outer<Out> res;
		for (auto&& inner : vec) for (auto&& e : inner)
			res.push_back(e);
		return res;
	}

	template <template<typename, typename...> typename ForwardIterable, typename In, typename Fn>
	inline auto flatmap(const ForwardIterable<In>& vec, const Fn& fn)
	{
		using Inner = std::decay_t<decltype(fn(*vec.begin()))>;
		using Out = std::decay_t<typename Inner::value_type>;
		ForwardIterable<Out> res;
		for (auto&& e : vec)
			for (auto&& e2 : fn(e))
				res.push_back(e2);
		return res;
	}

	template <template<typename, typename...> typename ForwardIterable, typename Fn>
	inline auto tabulate(const size_t n, const Fn& generator)
	{
		using Out = std::decay_t<decltype(generator(static_cast<size_t>(0)))>;
		ForwardIterable<Out> res;
		for (auto i = 0; i < n; ++i)
			res.push_back(generator(i));
		return res;
	}

	template <template<typename, typename...> typename ForwardIterable, typename Fn>
	inline auto fill(const size_t n, const Fn& generator)
	{
		using Out = std::decay_t<decltype(generator())>;
		ForwardIterable<Out> res;
		for (auto i = 0; i < n; ++i)
			res.push_back(generator());
		return res;
	}

	/** Returns a fmap with vec's values as keys
	*  and the results of fn applied to each key as values */
	template <typename Traversable, typename Fn,
		template <typename, typename, typename...> typename MapT = std::map>
	inline auto to_map(const Traversable& vec, const Fn& fn)
	{
		using In = typename Traversable::value_type;
		using Out = std::decay_t<decltype(fn(*vec.begin()))>;
		MapT<In, Out> res{};
		for (auto&& e : vec)
			res.insert({ e, fn(e) });
		return res;
	}

	/** Returns a map from the result of fn to a traversable of all elements,
	*  which when put through fn are equal to the key */
	template <template <typename, typename...> typename Traversable, typename In, typename Fn,
		template <typename, typename, typename...> typename MapT = std::map>
	auto group_by(const Traversable<In>& vec, const Fn& fn)
	{
		using Out = std::decay_t<decltype(fn(*vec.begin()))>;
		MapT<Out, Traversable<In>> res{};
		for (auto&& e : vec)
		{
			auto key = fn(e);
			auto search = res.find(key);
			if (search != res.end())
				res[key].push_back(e);
			else res[key] = Traversable<In>{ e };
		};
		return res;
	}

	template <template <typename, typename, typename...> typename MapT = map,
		typename Key, typename In, typename Fn>
		inline auto fmap_values(const MapT<Key, In> map, Fn fn)
	{
		using Out = std::decay_t<decltype(fn((*map.begin()).second))>;
		MapT<Key, Out> res;
		for (auto kv : map)
			res[kv.first] = fn(kv.second);
		return res;
	}

	template <template <typename, typename, typename...> typename MapT, typename Key, typename V>
	inline vector<Key> keys(const MapT<Key, V> map)
	{
		vector<Key> res{};
		res.reserve(map.size());
		for (auto&& e : map)
			res.push_back(e.first);
		return res;
	}

	template <template <typename, typename, typename...> typename MapT, typename Key, typename V>
	inline vector<V> values(const MapT<Key, V> map)
	{
		vector<V> res{};
		res.reserve(map.size());
		for (auto&& e : map)
			res.push_back(e.second);
		return res;
	}

	// TODO: make groups and slides lazy forward iterators

	template <size_t length, template <typename, typename...> typename Traversable, typename In>
	Traversable<Traversable<In>> grouped_copy(Traversable<In> vec)
	{
		Traversable<Traversable<In>> res{};
		auto it = vec.begin();
		while (it != vec.end())
		{
			Traversable<In> inner{};
			for (size_t i = 0; it != vec.end() && i < length; ++i, ++it)
				inner.push_back(*it);
			res.push_back(inner);
		}
		return res;
	}

	template <size_t length, template <typename, typename...> typename Traversable, typename In>
	Traversable<Traversable<In>> sliding_copy(Traversable<In> vec)
	{
		Traversable<Traversable<In>> res{};
		auto it = vec.begin();
		for (size_t index = 0; index + length - 1 < vec.size(); ++it, ++index)
		{
			Traversable<In> inner{};
			auto tmp_it = it;
			for (size_t i = 0; i < length; ++i) {
				inner.push_back(*tmp_it);
				++tmp_it;
			}
			res.push_back(inner);
		}
		return res;
	}

	// ___ TO STRING UTILS ___


	template <typename Writable>
	ostream& operator<<(ostream& os, const vector<Writable>& vec)
	{
		return os << show(vec);
	}

	template <typename A, typename B>
	ostream& operator<<(ostream& os, const pair<A, B>& pair)
	{
		return os << "(" << pair.first << " -> " << pair.second << ")";
	}
}
