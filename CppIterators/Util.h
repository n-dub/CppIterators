#pragma once
#include <list>
#include <iostream>
#include <vector>
#include <tuple>

template<class Tuple, size_t FI, size_t... I>
void PrintTupleImpl(std::ostream& stream, Tuple& tuple, std::index_sequence<FI, I...> seq, bool first = false) {
	if (first) {
		stream << "(";
	}
	stream << std::get<FI>(tuple);
	if constexpr (sizeof...(I) != 0) {
		stream << ", ";
		PrintTupleImpl(stream, tuple, std::index_sequence<I...>{});
	}
	else {
		stream << ")";
	}
}

template<class Tuple>
void PrintTupleImpl(std::ostream& stream, Tuple&, std::index_sequence<>, bool) {
	stream << "()";
}

template<class... Types>
std::ostream& operator<<(std::ostream& stream, std::tuple<Types...>&& tuple) {
	PrintTupleImpl(stream, tuple, std::index_sequence_for<Types...>{}, true);
	return stream;
}

template<class... Types>
std::ostream& operator<<(std::ostream& stream, std::tuple<Types...>& tuple) {
	PrintTupleImpl(stream, tuple, std::index_sequence_for<Types...>{}, true);
	return stream;
}
