#pragma once
#include "SDIterator.h"
#include "DDIterator.h"

namespace Iter
{
	template<class Iter, bool FCount = false>
	struct ForwardIterTrait
	{
		Iter iter, end;
		using Type = typename Iter::value_type;
		static inline constexpr bool FastCount = FCount;

		constexpr inline size_t Count() const noexcept {
			if constexpr (FastCount) {
				return std::distance(iter, end);
			}
			return 0;
		}

		constexpr inline ForwardIterTrait(Iter b, Iter e)
			: iter(b), end(e) { }

		constexpr inline std::optional<Type> Next() {
			if (iter == end)
				return {};
			return Type(*iter++);
		}
	};

	template<class Iter, bool FCount = false>
	struct ForwardRefIterTrait
	{
		Iter iter, end;
		using Type = Ref<typename Iter::value_type>;
		static inline constexpr bool FastCount = FCount;

		constexpr inline size_t Count() const noexcept {
			if constexpr (FastCount) {
				return std::distance(iter, end);
			}
			return 0;
		}

		constexpr inline ForwardRefIterTrait(Iter b, Iter e)
			: iter(b), end(e) { }

		constexpr inline std::optional<Type> Next() {
			if (iter == end)
				return {};
			return Type(*iter++);
		}
	};

	template<class Iter, bool FCount>
	struct DoubleDirIterTrait
	{
		Iter begin, end;
		using Type = typename Iter::value_type;
		static inline constexpr bool FastCount = FCount;

		constexpr inline size_t Count() const noexcept {
			if constexpr (FastCount) {
				return std::distance(begin, end);
			}
			return 0;
		}

		constexpr inline DoubleDirIterTrait(Iter b, Iter e)
			: begin(b), end(e) { }

		constexpr inline std::optional<Type> Next() {
			if (begin == end)
				return {};
			return Type(*begin++);
		}

		constexpr inline std::optional<Type> NextBack() {
			if (begin == end)
				return {};
			return Type(*++end);
		}
	};

	template<class Iter, bool FCount = false>
	struct DoubleDirRefIterTrait
	{
		Iter begin, end;
		using Type = Ref<typename Iter::value_type>;
		static inline constexpr bool FastCount = FCount;

		constexpr inline size_t Count() const noexcept {
			if constexpr (FastCount) {
				return std::distance(begin, end);
			}
			return 0;
		}

		constexpr inline DoubleDirRefIterTrait(Iter b, Iter e)
			: begin(b), end(e) { }

		constexpr inline std::optional<Type> Next() {
			if (begin == end)
				return {};
			return Type(*begin++);
		}

		constexpr inline std::optional<Type> NextBack() {
			if (begin == end)
				return {};
			return Type(*++end);
		}
	};

	template<class T>
	constexpr inline auto From(std::forward_list<T>& list) {
		auto trait = ForwardIterTrait{ list.begin(), list.end() };
		return SDIterator(trait);
	}

	template<class T>
	constexpr inline auto FromRef(std::forward_list<T>& list) {
		auto trait = ForwardRefIterTrait{ list.begin(), list.end() };
		return SDIterator(trait);
	}

	template<class T>
	constexpr inline auto From(std::list<T>& list) {
		auto trait = DoubleDirIterTrait<decltype(list.begin()), true>{ list.begin(), list.end() };
		return DDIterator(trait);
	}

	template<class T>
	constexpr inline auto FromRef(std::list<T>& list) {
		auto trait = DoubleDirRefIterTrait<decltype(list.begin()), true>{ list.begin(), list.end() };
		return DDIterator(trait);
	}
}
