#include <list>
#include <iostream>
#include <vector>
#include "Iterator.h"

template<class T1, class T2>
std::ostream& operator<<(std::ostream& stream, std::tuple<T1, T2> tuple) {
	return stream << "(" << std::get<0>(tuple) << ", " << std::get<1>(tuple) << ")";
}

int main() {
	for (auto x : Iter::Repeat("abc", 10)) {
		std::cout << x << "\n";
	}
}
