#pragma once
#include "IteratorCommon.h"

namespace Iter
{
	// SDTrait must be:
	// struct {
	//     using Type = ...;
	//     std::optional<Type> Next() { ... } // return current and move next
	// };
	template<class SDTrait>
	class SDIterator
	{
		SDTrait m_trait;
	public:
		using Type = typename SDTrait::Type;

		constexpr inline SDIterator(SDTrait t) : m_trait(t) { }

		inline constexpr std::optional<Type> Next() {
			return m_trait.Next();
		}

		inline auto begin() {
			return RangeForIter<SDIterator<SDTrait>>(*this);
		}

		inline auto end() {
			return RangeForIter<SDIterator<SDTrait>>(*this, false);
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

		inline constexpr auto Sum() const noexcept {
			return Fold([](auto a, auto b) { return a + b; });
		}

		inline constexpr auto Product() const noexcept {
			return Fold([](auto a, auto b) { return a * b; });
		}

		template<class Func>
		inline constexpr auto Map(Func func) const noexcept;

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
	struct ForwardRangeIterTrait
	{
		T begin, end;
		using Type = T;

		constexpr inline ForwardRangeIterTrait(T b, T e)
			: begin(b), end(e) { }

		constexpr inline std::optional<T> Next() {
			if (begin == end)
				return {};
			return begin++;
		}
	};

	template<class T1, class T2>
	constexpr inline auto FwdRange(T1 begin, T2 end) {
		static_assert(std::is_convertible<T2, T1>::value);
		static_assert(std::is_integral<T1>::value);
		auto trait = ForwardRangeIterTrait<T1>{ begin, T1(end) };
		return SDIterator(trait);
	}

	template<class SDTrait>
	inline constexpr auto SDIterator<SDTrait>::Enumerate() const noexcept {
		return FwdRange(size_t(0), size_t(-1)).Zip(*this);
	}

	template<class SDTrait>
	inline constexpr std::optional<typename SDIterator<SDTrait>::Type> SDIterator<SDTrait>::Nth(size_t n) const noexcept {
		auto it = this->Skip(n);
		return it.Next();
	}

	template<class Iter>
	inline constexpr auto SDSkipImpl(Iter it, size_t n) {
		while (n-- && it.Next());
		return it;
	}

	template<class SDTrait>
	inline constexpr auto SDIterator<SDTrait>::Skip(size_t n) const noexcept {
		return SDSkipImpl(*this, n);
	}

	template<class Iter>
	struct SDTakeIterTrait
	{
		Iter iter;
		size_t n;
		using Type = typename Iter::Type;
		constexpr inline SDTakeIterTrait(Iter i, size_t n) : iter(i), n(n) { }

		constexpr inline std::optional<Type> Next() {
			if (n == 0)
				return {};
			--n;
			return iter.Next();
		}
	};

	template<class Iter>
	inline constexpr auto SDTakeImpl(Iter it, size_t n) {
		return SDIterator(SDTakeIterTrait<Iter>{ it, n });
	}

	template<class SDTrait>
	inline constexpr auto SDIterator<SDTrait>::Take(size_t n) const noexcept {
		return SDTakeImpl(*this, n);
	}

	template<class Iter>
	struct SDStepByIterTrait
	{
		Iter iter;
		size_t n, idx = 0;
		using Type = typename Iter::Type;
		constexpr inline SDStepByIterTrait(Iter i, size_t n) : iter(i), n(n) { }

		constexpr inline auto Next() {
			while (idx) {
				idx = (idx + 1) % n;
				if (!iter.Next())
					break;
			}
			idx = (idx + 1) % n;
			return iter.Next();
		}
	};

	template<class Iter>
	inline constexpr auto SDStepByImpl(Iter it, size_t n) {
		return SDIterator(SDStepByIterTrait{ it, n });
	}

	template<class SDTrait>
	inline constexpr auto Iter::SDIterator<SDTrait>::StepBy(size_t n) const noexcept {
		return SDStepByImpl(*this, n);
	}

	template<class T1, class T2>
	struct SDZipIterTrait
	{
		SDIterator<T1> iter1;
		SDIterator<T2> iter2;
		using Type = std::tuple<typename T1::Type, typename T2::Type>;

		constexpr inline SDZipIterTrait(SDIterator<T1> l, SDIterator<T2> r) : iter1(l), iter2(r) { }

		constexpr inline std::optional<Type> Next() {
			if (auto next1 = iter1.Next()) {
				if (auto next2 = iter2.Next()) {
					return std::make_tuple(next1.value(), next2.value());
				}
			}

			return {};
		}
	};

	template<class T1, class T2>
	inline constexpr auto SDZipImpl(SDIterator<T1> l, SDIterator<T2> r) {
		return SDIterator(SDZipIterTrait{ l, r });
	}

	template<class SDTrait>
	template<class Iter>
	inline constexpr auto SDIterator<SDTrait>::Zip(Iter other) const noexcept {
		return SDZipImpl(*this, other);
	}

	template<class T1, class T2>
	struct SDChainIterTrait
	{
		static_assert(std::is_same<typename T1::Type, typename T2::Type>::value, "Chained iterators must have same value type");
		SDIterator<T1> iter1;
		SDIterator<T2> iter2;
		using Type = typename T1::Type;

		constexpr inline SDChainIterTrait(SDIterator<T1> l, SDIterator<T2> r) : iter1(l), iter2(r) { }

		constexpr inline std::optional<Type> Next() {
			if (auto next = iter1.Next()) {
				return next.value();
			}

			if (auto next = iter2.Next()) {
				return next.value();
			}

			return {};
		}
	};

	template<class T1, class T2>
	inline constexpr auto SDChainImpl(SDIterator<T1> l, SDIterator<T2> r) {
		return SDIterator(SDChainIterTrait{ l, r });
	}

	template<class SDTrait>
	template<class Iter>
	inline constexpr auto SDIterator<SDTrait>::Chain(Iter other) const noexcept {
		return SDChainImpl(*this, other);
	}

	template<class T, class Func, class Ret>
	struct SDMapIterTrait
	{
		SDIterator<T> iter;
		Func func;
		using Type = typename Ret;

		constexpr inline SDMapIterTrait(SDIterator<T> it, Func f) : iter(it), func(f) { }

		constexpr inline std::optional<Ret> Next() {
			if (auto next = iter.Next()) {
				return func(next.value());
			}
			return {};
		}
	};

	template<class T, class Func>
	inline constexpr auto SDMapImpl(SDIterator<T> it, Func f) {
		return SDIterator(SDMapIterTrait<T, Func, decltype(f(it.Next().value()))>{ it, f });
	}

	template<class SDTrait>
	template<class Func>
	inline constexpr auto SDIterator<SDTrait>::Map(Func func) const noexcept {
		return SDMapImpl(*this, func);
	}

	template<class T>
	struct SDRevIterTrait
	{
		using Type = typename SDIterator<T>::Type;
		std::vector<Type> values;
		size_t idx;

		constexpr inline SDRevIterTrait(SDIterator<T> it) {
			while (auto v = it.Next()) {
				values.push_back(v.value());
			}
			idx = values.size();
		}

		constexpr inline std::optional<Type> Next() {
			if (idx > 0)
				return values[--idx];
			return {};
		}
	};

	template<class T>
	inline constexpr auto SDRevImpl(SDIterator<T> it) {
		return SDIterator(SDRevIterTrait{ it });
	}

	template<class SDTrait>
	inline constexpr auto Iter::SDIterator<SDTrait>::Reverse() const noexcept {
		return SDRevImpl(*this);
	}
}
