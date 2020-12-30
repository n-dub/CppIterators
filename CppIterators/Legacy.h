#pragma once
#include <optional>
#include <tuple>
#include <limits>
#include <stdint.h>

namespace Iter
{
	template<class Iter>
	class RangeForIter
	{
		Iter m_iter;
		std::optional<typename Iter::Type> m_curr;
		bool m_valid;

	public:
		inline constexpr RangeForIter(Iter it, bool valid = true)
			: m_iter(it)
		{
			m_valid = valid;
			m_curr = m_iter.Next();
		}

		inline constexpr auto& operator++() {
			m_curr = m_iter.Next();
			return *this;
		}

		inline constexpr auto operator*() {
			return m_curr.value();
		}

		inline constexpr bool Valid() {
			return m_curr.has_value() && m_valid;
		}

		inline constexpr bool operator!=(RangeForIter other) {
			return other.Valid() != Valid();
		}
	};

	template<class Data, class FNext>
	class Iterator
	{
		FNext m_nextFunc;
		Data m_data;
	public:
		using Type = typename Data::Type;

		inline constexpr Iterator(FNext next, Data data)
			: m_data(data), m_nextFunc(next) { }

		inline constexpr auto Next() {
			return m_nextFunc(m_data);
		}

		inline constexpr auto begin() {
			return RangeForIter(*this);
		}

		inline constexpr auto end() {
			return RangeForIter(*this, false);
		}

		inline constexpr auto Skip(size_t n) {
			auto res = *this;
			while (res.Next() && --n);
			return res;
		}

		inline constexpr auto Take(size_t n);

		template<class Iter2>
		inline constexpr auto Zip(Iter2 other);

		inline constexpr auto Enumerate(size_t start = 0);

		inline constexpr auto StepBy(size_t n);

		inline constexpr std::optional<typename Data::Type> Nth(size_t n) {
			while (auto x = Next()) {
				if (--n == 0)
					return x;
			}
			return {};
		}

		template<class Iter2>
		inline constexpr auto Chain(Iter2 other);

		template<class Func>
		inline constexpr auto Fold(Type init, Func func) {
			while (auto v = Next()) {
				init = func(init, v.value());
			}
			return init;
		}

		template<class Func>
		inline constexpr auto Fold(Func func) {
			auto init = Next().value();
			return Fold(init, func);
		}

		inline constexpr auto Sum() {
			return Fold([](Type a, Type b) { return a + b; });
		}

		inline constexpr auto Product() {
			return Fold([](Type a, Type b) { return a * b; });
		}

		template<class Func>
		inline constexpr auto Map(Func func);

		template<class OutputIt>
		inline constexpr void Collect(OutputIt first) {
			while (auto val = Next()) {
				*first++ = val.value();
			}
		}

		template<class Container>
		inline constexpr Container Collect() {
			Container c{};
			Collect(std::back_inserter(c));
			return c;
		}
	};

	template<class It, class Func, class FResult>
	struct MapIterData
	{
		using Type = FResult;
		It iter;
		Func func;

		inline constexpr MapIterData(It iter, Func func)
			: iter(iter), func(func) { }
	};

	template<class Iter, class Func>
	inline constexpr auto Map(Iter it, Func func) {
		auto l = [func](MapIterData<Iter, Func, decltype(func(it.Next().value()))>& d) -> std::optional<decltype(func(it.Next().value()))>
		{
			if (auto v = d.iter.Next()) {
				return func(v.value());
			}
			return {};
		};
		return Iterator(l, MapIterData<Iter, Func, decltype(func(it.Next().value()))>(it, func));
	}

	template<class Data, class FNext>
	template<class Func>
	inline constexpr auto Iter::Iterator<Data, FNext>::Map(Func func) {
		return ::Iter::Map(*this, func);
	}

	template<class It>
	struct StepIterData
	{
		using Type = typename It::Type;
		It iter;
		size_t n, idx;

		inline constexpr StepIterData(It iter, size_t n)
			: iter(iter), n(n), idx(0) { }
	};

	template<class Iter>
	inline constexpr auto StepBy(Iter it, size_t n) {
		auto l = [](StepIterData<Iter>& d) -> std::optional<typename Iter::Type>
		{
			while (d.idx) {
				d.idx = (d.idx + 1) % d.n;
				if (!d.iter.Next())
					break;
			}
			d.idx = (d.idx + 1) % d.n;
			return d.iter.Next();
		};
		return Iterator(l, StepIterData<Iter>(it, n));
	}

	template<class Data, class FNext>
	inline constexpr auto Iterator<Data, FNext>::StepBy(size_t n) {
		return ::Iter::StepBy(*this, n);
	}

	template<class It1, class It2>
	struct ChainIterData
	{
		using Type = typename It1::Type;
		It1 iter1;
		It2 iter2;

		inline constexpr ChainIterData(It1 iter1, It2 iter2)
			: iter1(iter1), iter2(iter2) { }
	};

	template<class Iter1, class Iter2>
	inline constexpr auto Chain(Iter1 it1, Iter2 it2) {
		auto l = [](ChainIterData<Iter1, Iter2>& d) -> std::optional<typename Iter1::Type>
		{
			if (auto v = d.iter1.Next()) {
				return v.value();
			}
			if (auto v = d.iter2.Next()) {
				return v.value();
			}

			return {};
		};
		return Iterator(l, ChainIterData<Iter1, Iter2>(it1, it2));
	}

	template<class Data, class FNext>
	template<class Iter2>
	inline constexpr auto Iter::Iterator<Data, FNext>::Chain(Iter2 other) {
		return ::Iter::Chain(*this, other);
	}

	template<class It1, class It2>
	struct ZipIterData
	{
		using Type = std::tuple<typename It1::Type, typename It2::Type>;
		It1 iter1;
		It2 iter2;

		inline constexpr ZipIterData(It1 iter1, It2 iter2)
			: iter1(iter1), iter2(iter2) { }
	};

	template<class Iter1, class Iter2>
	inline constexpr auto Zip(Iter1 it1, Iter2 it2) {
		auto l = [](ZipIterData<Iter1, Iter2>& d) -> std::optional<std::tuple<typename Iter1::Type, typename Iter2::Type>>
		{
			if (auto next1 = d.iter1.Next()) {
				if (auto next2 = d.iter2.Next()) {
					return std::make_tuple(next1.value(), next2.value());
				}
			}

			return {};
		};
		return Iterator(l, ZipIterData<Iter1, Iter2>(it1, it2));
	}

	template<class Data, class FNext>
	template<class Iter2>
	inline constexpr auto Iterator<Data, FNext>::Zip(Iter2 other) {
		return ::Iter::Zip(*this, other);
	}

	template<class It>
	struct TakeIterData
	{
		using Type = typename It::Type;
		It iter;
		size_t n;

		inline constexpr TakeIterData(It iter, size_t n)
			: iter(iter), n(n) { }
	};

	template<class Iter>
	inline constexpr auto Take(Iter it, size_t n) {
		auto l = [](TakeIterData<Iter>& d) -> std::optional<typename Iter::Type> {
			if (d.n-- <= 0)
				return {};
			return d.iter.Next();
		};
		return Iterator(l, TakeIterData<Iter>(it, n));
	}

	template<class Data, class FNext>
	inline constexpr auto Iterator<Data, FNext>::Take(size_t n) {
		return ::Iter::Take(*this, n);
	}

	template<class It, class T>
	struct StdIterData
	{
		using Type = T;
		It iter, end;

		inline constexpr StdIterData(It b, It e) {
			iter = b;
			end = e;
		}
	};

	template<class It>
	inline constexpr auto From(It begin, It end) {
		return Iterator(
			[](StdIterData<It, typename std::remove_reference<decltype(*begin)>::type>& d) ->
				std::optional<typename std::remove_reference<decltype(*begin)>::type>
			{
				if (d.iter == d.end)
					return {};
				return *d.iter++;
			},
			StdIterData<It, typename std::remove_reference<decltype(*begin)>::type>(begin, end));
	}

	template<class T>
	inline constexpr auto From(T& cont) {
		return From(cont.begin(), cont.end());
	}

	template<class T, size_t N>
	inline constexpr auto From(T(&arr)[N]) {
		return From(arr, arr + N);
	}

	template<class T>
	struct OnceIterData
	{
		using Type = T;
		T element;
		bool valid;

		OnceIterData(T e) : element(e), valid(true) { }
	};

	template<class T>
	inline constexpr auto Once(T elem) {
		return Iterator(
			[](OnceIterData<T>& d) -> std::optional<T> {
				if (d.valid) {
					d.valid = false;
					return d.element;
				}
				return {};
			},
			OnceIterData<T>(elem));
	}

	template<class T>
	struct RepIterData
	{
		using Type = T;
		T element;

		RepIterData(T e) : element(e) { }
	};

	template<class T>
	inline constexpr auto Repeat(T elem) {
		return Iterator(
			[](RepIterData<T>& d) -> std::optional<T> {
				return d.element;
			},
			RepIterData<T>(elem));
	}

	template<class T>
	inline constexpr auto Repeat(T elem, size_t n) {
		return Repeat(elem).Take(n);
	}

	template<class T>
	struct NumIterData
	{
		using Type = T;
		T n, end;

		NumIterData(T b, T e) {
			n = b;
			end = e;
		}
	};

	template<class T>
	inline constexpr auto Range(T begin, T end = std::numeric_limits<T>::max()) {
		return Iterator(
			[](NumIterData<T>& d) -> std::optional<T> {
				if (d.n == d.end)
					return {};
				return d.n++;
			},
			NumIterData<T>(begin, end));
	}

#pragma region ranges

	template<>
	inline auto From(int64_t begin, int64_t end) {
		return Range(begin, end);
	}

	template<>
	inline auto From(int32_t begin, int32_t end) {
		return Range(begin, end);
	}

	template<>
	inline auto From(int16_t begin, int16_t end) {
		return Range(begin, end);
	}

	template<>
	inline auto From(int8_t begin, int8_t end) {
		return Range(begin, end);
	}

	template<>
	inline auto From(uint64_t begin, uint64_t end) {
		return Range(begin, end);
	}

	template<>
	inline auto From(uint32_t begin, uint32_t end) {
		return Range(begin, end);
	}

	template<>
	inline auto From(uint16_t begin, uint16_t end) {
		return Range(begin, end);
	}

	template<>
	inline auto From(uint8_t begin, uint8_t end) {
		return Range(begin, end);
	}
#pragma endregion

	template<class Data, class FNext>
	inline constexpr auto Iterator<Data, FNext>::Enumerate(size_t start) {
		return Range(start).Zip(*this);
	}
}
