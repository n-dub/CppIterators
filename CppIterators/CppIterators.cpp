#include <forward_list>
#include "Iterator.h"
#include "Util.h"

int main() {
	std::forward_list<int> a{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	std::forward_list<double> b{ 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };

	for (auto [d, i] : Iter::FromRef(b).Zip(Iter::From(a))) {
		d /= 100;
	}

	for (auto t : Iter::From(b).Zip(Iter::From(a))) {
		std::cout << t << " ";
	}

	std::cout << "\n";
	for (auto t : Iter::From(a).Skip(2).StepBy(2).Take(3)) {
		std::cout << t << " ";
	}

	std::cout << "\n";
	for (int i = -3; i < 15; ++i)
		std::cout << Iter::From(b).Nth(i).value_or(-999999) << " ";

	std::cout << "\n";
	for (auto t : Iter::From(a).Chain(Iter::From(a))) {
		std::cout << t << " ";
	}

	std::cout << "\n";
	for (auto [v, i] : Iter::FromRef(a).Enumerate()) {
		std::cout << i << ": " << v << "\n";
		v = rand();
		std::cout << i << ": " << v << "\n";
	}

	std::cout << "\n";
	for (auto v : Iter::From(a)) {
		std::cout << v << " ";
	}

	std::cout << "\nSum of a: " << Iter::From(a).Fold(0.0, [](auto a, auto b) { return double(a) + double(b); }) << "\n";

	auto half_a = Iter::From(a).Take(5).ToVector();

	for (auto t : Iter::From(half_a)) {
		std::cout << t << " ";
	}

	std::cout << "\n5! = " << Iter::FwdRange(1, 6).Product() << "\n";
	for (auto t : Iter::FwdRange(0, 10).Map([](auto x) { return x * x; }).Enumerate()) {
		std::cout << t << " ";
	}

	std::cout << "\n";
	for (auto v : Iter::FwdRange(0, 10).Reverse()) {
		std::cout << v << " ";
	}
}
