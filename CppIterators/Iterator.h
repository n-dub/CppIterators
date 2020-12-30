#pragma once
#include <forward_list>
#include <vector>
#include "SDIterator.h"

namespace Iter
{
	template<class Iter>
	struct ForwardIterTrait
	{
		Iter iter, end;
		using Type = typename Iter::value_type;

		ForwardIterTrait(Iter b, Iter e)
			: iter(b), end(e) { }

		std::optional<Type> Next() {
			if (iter == end)
				return {};
			return *iter++;
		}
	};

	template<class Iter>
	struct ForwardRefIterTrait
	{
		Iter iter, end;
		using NonRefType = typename Iter::value_type;
		using Type = Ref<typename Iter::value_type>;

		ForwardRefIterTrait(Iter b, Iter e)
			: iter(b), end(e) { }

		std::optional<Ref<NonRefType>> Next() {
			if (iter == end)
				return {};
			return Ref<NonRefType>(*iter++);
		}
	};

	template<class T>
	constexpr inline auto From(std::forward_list<T>& list) {
		auto trait = ForwardIterTrait{ list.begin(), list.end() };
		return SDIterator(trait);
	}

	template<class T>
	constexpr inline auto From(std::vector<T>& list) {
		auto trait = ForwardIterTrait{ list.begin(), list.end() };
		return SDIterator(trait);
	}

	template<class T>
	constexpr inline auto FromRef(std::forward_list<T>& list) {
		auto trait = ForwardRefIterTrait{ list.begin(), list.end() };
		return SDIterator(trait);
	}

	template<class T>
	constexpr inline auto FromRef(std::vector<T>& list) {
		auto trait = ForwardRefIterTrait{ list.begin(), list.end() };
		return SDIterator(trait);
	}
}
