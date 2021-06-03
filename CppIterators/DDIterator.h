#pragma once
#include "IteratorCommon.h"

namespace Iter
{
	/*
	 * SDTrait must be:
	 * struct {
	 *     using Type = ...;
	 *
	 *     // true if Count() works
	 *     static inline constexpr bool FastCount = ...;
	 *
	 *     // return length of the iterator with O(1) complexity if possible
	 *     size_t Count()             { ... }
	 *
	 *     // return front and move forward
	 *     std::optional<Type> Next() { ... }
	 * 
	 * 	   // return back and move backwards
	 *     std::optional<Type> NextBack() { ... }
	 * };
	*/
	template<class DDTrait>
	class DDIterator
	{
		DDTrait m_trait;
	public:
		using Type = typename DDTrait::Type;
		static inline constexpr bool FastCount = DDTrait::FastCount;

		constexpr inline DDIterator(DDTrait t) : m_trait(t) { }

		inline constexpr std::optional<Type> Next() noexcept {
			return m_trait.Next();
		}

		inline constexpr std::optional<Type> NextBack() noexcept {
			return m_trait.NextBack();
		}

		inline auto begin() const noexcept {
			return RangeForIter<DDIterator<DDTrait>>(*this);
		}

		inline auto end() const noexcept {
			return RangeForIter<DDIterator<DDTrait>>(*this, false);
		}

		inline constexpr auto Skip(size_t n) const noexcept;
		inline constexpr auto Take(size_t n) const noexcept;
		inline constexpr auto StepBy(size_t n) const noexcept;
		inline constexpr std::optional<Type> Nth(size_t n) const noexcept;

		template<class Iter>
		inline constexpr auto Zip(Iter other) const noexcept;
		template<class Iter>
		inline constexpr auto Chain(Iter other) const noexcept;

		inline constexpr auto Enumerate() const noexcept;

		inline constexpr auto Reverse() const noexcept;

		template<class Func, class Ret>
		inline constexpr Ret Fold(Ret init, Func func) const noexcept {
			auto it = *this;
			while (auto v = it.Next()) {
				init = func(init, v.value());
			}
			return init;
		}

		template<class Func>
		inline constexpr auto Fold(Func func) const noexcept {
			auto it = *this;
			auto init = it.Next().value();
			while (auto v = it.Next()) {
				init = func(init, v.value());
			}
			return init;
		}

		inline constexpr size_t Count() const noexcept {
			if constexpr (DDTrait::FastCount) {
				return m_trait.Count();
			}
			return Fold(size_t(0),
				[](auto a, auto) {
					return a + 1;
				}
			);
		}

		inline constexpr auto Sum() const noexcept {
			return Fold([](auto a, auto b) { return a + b; });
		}

		inline constexpr auto Product() const noexcept {
			return Fold([](auto a, auto b) { return a * b; });
		}

		template<class Func>
		inline constexpr auto Map(Func func) const noexcept;

		template<class Func>
		inline constexpr auto Filter(Func func) const noexcept;

		template<class Cont>
		inline constexpr auto Collect() const noexcept {
			auto it = *this;
			Cont cont{};
			while (auto v = it.Next()) {
				cont.push_back(v.value());
			}
			return cont;
		}

		inline constexpr auto ToVector() const noexcept {
			return Collect<std::vector<Type>>();
		}
	};

	template<class T>
	struct DoubleDirRangeIterTrait
	{
		T begin, end;
		using Type = T;
		static inline constexpr bool FastCount = true;

		constexpr inline size_t Count() const noexcept {
			return size_t(end) - size_t(begin);
		}

		constexpr inline DoubleDirRangeIterTrait(T b, T e)
			: begin(b), end(e) { }

		constexpr inline std::optional<T> Next() {
			if (begin == end)
				return {};
			return begin++;
		}

		constexpr inline std::optional<T> NextBack() {
			if (begin == end)
				return {};
			return --end;
		}
	};

	template<class T1, class T2>
	constexpr inline auto DDRange(T1 begin, T2 end) {
		static_assert(std::is_convertible<T2, T1>::value);
		static_assert(std::is_integral<T1>::value);
		auto trait = DoubleDirRangeIterTrait<T1>{ begin, T1(end) };
		return DDIterator(trait);
	}

	template<class DDTrait>
	inline constexpr auto DDIterator<DDTrait>::Enumerate() const noexcept {
		return DDRange(size_t(0), size_t(-1)).Zip(*this);
	}

	template<class DDTrait>
	inline constexpr std::optional<typename DDIterator<DDTrait>::Type> DDIterator<DDTrait>::Nth(size_t n) const noexcept {
		auto it = this->Skip(n);
		return it.Next();
	}

	template<class Iter>
	inline constexpr auto DDSkipImpl(Iter it, size_t n) {
		while (n-- && it.Next());
		return it;
	}

	template<class DDTrait>
	inline constexpr auto DDIterator<DDTrait>::Skip(size_t n) const noexcept {
		return DDSkipImpl(*this, n);
	}

	template<class Iter>
	struct DDTakeIterTrait
	{
		Iter iter;
		using Type = typename Iter::Type;
		static inline constexpr bool FastCount = Iter::FastCount;

		constexpr inline size_t Count() const noexcept {
			if constexpr (FastCount) {
				return iter.Count();
			}
			return 0;
		}

		// TODO: this is not very lazy
		constexpr inline DDTakeIterTrait(Iter i, size_t n) : iter(i) {
			size_t len = iter.Count();
			if (len < n)
				return;
			size_t skipBack = len - n;
			while (skipBack-- && iter.NextBack());
		}

		constexpr inline std::optional<Type> Next() {
			return iter.Next();
		}

		constexpr inline std::optional<Type> NextBack() {
			return iter.NextBack();
		}
	};

	template<class Iter>
	inline constexpr auto DDTakeImpl(Iter it, size_t n) {
		return DDIterator(DDTakeIterTrait<Iter>{ it, n });
	}

	template<class DDTrait>
	inline constexpr auto DDIterator<DDTrait>::Take(size_t n) const noexcept {
		return DDTakeImpl(*this, n);
	}

	template<class Iter>
	struct DDStepByIterTrait
	{
		Iter iter;
		size_t n;
		using Type = typename Iter::Type;
		static inline constexpr bool FastCount = Iter::FastCount;

		constexpr inline size_t Count() const noexcept {
			if constexpr (FastCount) {
				auto size = iter.Count();
				return size / n + size % n;
			}
			return 0;
		}

		constexpr inline DDStepByIterTrait(Iter i, size_t n) : iter(i), n(n) { }

		constexpr inline auto Next() {
			auto v = iter.Next();
			Skip(n - 1);
			return v;
		}

		constexpr inline auto NextBack() {
			auto v = iter.NextBack();
			SkipBack(n - 1);
			return v;
		}

	private:
		constexpr inline void Skip(size_t n) {
			while (n-- && iter.Next());
		}

		constexpr inline void SkipBack(size_t n) {
			while (n-- && iter.NextBack());
		}
	};

	template<class Iter>
	inline constexpr auto DDStepByImpl(Iter it, size_t n) {
		return DDIterator(DDStepByIterTrait{ it, n });
	}

	template<class DDTrait>
	inline constexpr auto Iter::DDIterator<DDTrait>::StepBy(size_t n) const noexcept {
		return DDStepByImpl(*this, n);
	}

	template<class T1, class T2>
	struct DDZipIterTrait
	{
		DDIterator<T1> iter1;
		DDIterator<T2> iter2;
		using Type = std::tuple<typename T1::Type, typename T2::Type>;
		static inline constexpr bool FastCount = T1::FastCount && T2::FastCount;

		constexpr inline size_t Count() const noexcept {
			if constexpr (FastCount) {
				return std::min(iter1.Count(), iter2.Count());
			}
			return 0;
		}

		constexpr inline DDZipIterTrait(DDIterator<T1> l, DDIterator<T2> r) : iter1(l), iter2(r) { }

		constexpr inline std::optional<Type> Next() {
			if (auto next1 = iter1.Next()) {
				if (auto next2 = iter2.Next()) {
					return std::make_tuple(next1.value(), next2.value());
				}
			}

			return {};
		}

		constexpr inline std::optional<Type> NextBack() {
			if (auto next1 = iter1.NextBack()) {
				if (auto next2 = iter2.NextBack()) {
					return std::make_tuple(next1.value(), next2.value());
				}
			}

			return {};
		}
	};

	template<class T1, class T2>
	inline constexpr auto DDZipImpl(DDIterator<T1> l, DDIterator<T2> r) {
		return DDIterator(DDZipIterTrait{ l, r });
	}

	template<class DDTrait>
	template<class Iter>
	inline constexpr auto DDIterator<DDTrait>::Zip(Iter other) const noexcept {
		return DDZipImpl(*this, other);
	}

	template<class T1, class T2>
	struct DDChainIterTrait
	{
		static_assert(std::is_same<typename T1::Type, typename T2::Type>::value, "Chained iterators must have same value type");
		DDIterator<T1> iter1;
		DDIterator<T2> iter2;
		using Type = typename T1::Type;
		static inline constexpr bool FastCount = T1::FastCount && T2::FastCount;

		constexpr inline size_t Count() const noexcept {
			if constexpr (FastCount) {
				return iter1.Count() + iter2.Count();
			}
			return 0;
		}

		constexpr inline DDChainIterTrait(DDIterator<T1> l, DDIterator<T2> r) : iter1(l), iter2(r) { }

		constexpr inline std::optional<Type> Next() {
			if (auto next = iter1.Next()) {
				return next.value();
			}

			if (auto next = iter2.Next()) {
				return next.value();
			}

			return {};
		}

		constexpr inline std::optional<Type> NextBack() {
			if (auto next = iter2.NextBack()) {
				return next.value();
			}

			if (auto next = iter1.NextBack()) {
				return next.value();
			}

			return {};
		}
	};

	template<class T1, class T2>
	inline constexpr auto DDChainImpl(DDIterator<T1> l, DDIterator<T2> r) {
		return DDIterator(DDChainIterTrait{ l, r });
	}

	template<class DDTrait>
	template<class Iter>
	inline constexpr auto DDIterator<DDTrait>::Chain(Iter other) const noexcept {
		return DDChainImpl(*this, other);
	}

	template<class T, class Func, class Ret>
	struct DDMapIterTrait
	{
		DDIterator<T> iter;
		Func func;
		using Type = Ret;
		static inline constexpr bool FastCount = T::FastCount;

		constexpr inline size_t Count() const noexcept {
			if constexpr (FastCount) {
				return iter.Count();
			}
			return 0;
		}

		constexpr inline DDMapIterTrait(DDIterator<T> it, Func f) : iter(it), func(f) { }

		constexpr inline std::optional<Ret> Next() {
			if (auto next = iter.Next()) {
				return func(next.value());
			}
			return {};
		}

		constexpr inline std::optional<Ret> NextBack() {
			if (auto next = iter.NextBack()) {
				return func(next.value());
			}
			return {};
		}
	};

	template<class T, class Func>
	inline constexpr auto DDMapImpl(DDIterator<T> it, Func f) {
		return DDIterator(DDMapIterTrait<T, Func, decltype(f(it.Next().value()))>{ it, f });
	}

	template<class DDTrait>
	template<class Func>
	inline constexpr auto DDIterator<DDTrait>::Map(Func func) const noexcept {
		return DDMapImpl(*this, func);
	}

	template<class T, class Func>
	struct DDFilterIterTrait
	{
		DDIterator<T> iter;
		Func func;
		using Type = typename T::Type;
		static inline constexpr bool FastCount = T::FastCount;

		constexpr inline size_t Count() const noexcept {
			if constexpr (FastCount) {
				return iter.Count();
			}
			return 0;
		}

		constexpr inline DDFilterIterTrait(DDIterator<T> it, Func f) : iter(it), func(f) { }

		constexpr inline std::optional<Type> Next() {
			while (auto next = iter.Next()) {
				if (!func(next.value()))
					continue;
				return next.value();
			}
			return {};
		}

		constexpr inline std::optional<Type> NextBack() {
			while (auto next = iter.NextBack()) {
				if (!func(next.value()))
					continue;
				return next.value();
			}
			return {};
		}
	};

	template<class T, class Func>
	inline constexpr auto DDFilterImpl(DDIterator<T> it, Func f) {
		return DDIterator(DDFilterIterTrait<T, Func>{ it, f });
	}

	template<class DDTrait>
	template<class Func>
	inline constexpr auto DDIterator<DDTrait>::Filter(Func func) const noexcept {
		return DDFilterImpl(*this, func);
	}

	template<class T>
	struct DDRevIterTrait
	{
		using Type = typename DDIterator<T>::Type;
		static inline constexpr bool FastCount = T::FastCount;

		DDIterator<T> iter;

		constexpr inline size_t Count() const noexcept {
			if constexpr (FastCount) {
				return iter.Count();
			}
			return 0;
		}

		constexpr inline DDRevIterTrait(DDIterator<T> it) : iter(it) { }

		constexpr inline std::optional<Type> Next() {
			return iter.NextBack();
		}

		constexpr inline std::optional<Type> NextBack() {
			return iter.Next();
		}
	};

	template<class T>
	inline constexpr auto DDRevImpl(DDIterator<T> it) {
		return DDIterator(DDRevIterTrait{ it });
	}

	template<class DDTrait>
	inline constexpr auto Iter::DDIterator<DDTrait>::Reverse() const noexcept {
		return DDRevImpl(*this);
	}
}
