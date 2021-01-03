#pragma once
#include <optional>
#include <tuple>
#include <limits>
#include <stdint.h>
#include <forward_list>
#include <list>
#include <vector>

namespace Iter
{
	template<class... Params>
	constexpr auto DummyVoid = [](Params...) { };
	template<class R, R ret, class... Params>
	constexpr auto DummyFunc = [](Params...) -> R { return ret; };

	template<class T>
	class Ref
	{
		T* m_value;
	public:
		Ref() : m_value(nullptr) {}

		Ref(T& val) : m_value(&val) {}

		Ref& operator=(const T& val) {
			*m_value = val;
			return *this;
		}

		operator T&() {
			return *m_value;
		}
	};

	template<class Iter>
	class RangeForIter
	{
		using Type = typename Iter::Type;

		Iter m_iter;
		std::optional<Type> m_curr;
		bool m_valid;

	public:
		inline constexpr RangeForIter(Iter it, bool valid = true)
			: m_iter(it), m_valid(valid)
		{
			operator++();
		}

		inline constexpr auto& operator++() {
			m_curr = m_iter.Next();
			return *this;
		}

		inline constexpr Type operator*() {
			return m_curr.value();
		}

		inline constexpr bool Valid() {
			return m_curr && m_valid;
		}

		inline constexpr bool operator!=(RangeForIter other) {
			return other.Valid() != Valid();
		}
	};
}
