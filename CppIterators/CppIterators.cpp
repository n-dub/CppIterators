#include <vector>
#include <iostream>
#include "Iterator.h"

int main() {
	std::vector a{ 10, 1, 2, 3, 45, 99 };

	for (auto v : Iter::From(a)) {
		std::cout << v << "\n";
	}
}
