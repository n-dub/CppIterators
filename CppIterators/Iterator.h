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

		inline constexpr auto Enumerate();
	};

	template<class It1, class It2>
	struct EnumIterData
	{
		using Type = std::tuple<typename It1::Type, typename It2::Type>;
		It1 iter1;
		It2 iter2;

		inline constexpr EnumIterData(It1 iter1, It2 iter2)
			: iter1(iter1), iter2(iter2) { }
	};

	template<class Iter1, class Iter2>
	inline constexpr auto Zip(Iter1 it1, Iter2 it2) {
		auto l = [](EnumIterData<Iter1, Iter2>& d) -> std::optional<std::tuple<typename Iter1::Type, typename Iter2::Type>>
		{
			if (auto next1 = d.iter1.Next()) {
				if (auto next2 = d.iter2.Next()) {
					return std::make_tuple(next1.value(), next2.value());
				}
			}

			return {};
		};
		return Iterator(l, EnumIterData<Iter1, Iter2>(it1, it2));
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
		auto l = [](TakeIterData<Iter>& d) -> std::optional<typename Iter::Type>
		{
			if (d.n-- == 0)
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
			[](StdIterData<It, typename std::remove_reference<decltype(*begin)>::type>& d)
				-> std::optional<typename std::remove_reference<decltype(*begin)>::type>
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

	template<class T>
	struct ArrIterData
	{
		using Type = T;
		T *iter, *end;

		inline constexpr ArrIterData(T* b, T* e) {
			iter = b;
			end = e;
		}
	};

	template<class T, size_t N>
	inline constexpr auto From(T (&arr)[N]) {
		return From(arr, arr+N);
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

#pragma region ranges

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
	inline constexpr  auto Iterator<Data, FNext>::Enumerate() {
		return Range(size_t(0)).Zip(*this);
	}
}
