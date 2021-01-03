#include <forward_list>
#include "Iterator.h"
#include "Util.h"

int main() {
	std::forward_list<int> a{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	std::forward_list<double> b{ 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };

	std::cout << "all d in b: d /= 100\n";
	for (auto [d, i] : Iter::FromRef(b).Zip(Iter::From(a))) {
		d /= 100;
	}

	std::cout << "\nIter::From(b).Zip(Iter::From(a)):\n";
	for (auto t : Iter::From(b).Zip(Iter::From(a))) {
		std::cout << t << " ";
	}

	std::cout << "\n\nIter::From(a).Skip(2).StepBy(2).Take(3):\n";
	for (auto t : Iter::From(a).Skip(2).StepBy(2).Take(3)) {
		std::cout << t << " ";
	}

	std::cout << "\n\nSum of a as double: "
		<< Iter::From(a).Fold(0.0, [](auto a, auto b) { return double(a) + double(b); }) << "\n";

	std::cout << "\nfor n in [-3; 15): b[n]:\n";
	for (int i = -3; i < 15; ++i)
		std::cout << Iter::From(b).Nth(i).value_or(-999999) << " ";

	std::cout << "\n\nIter::From(a).Chain(Iter::From(a)):\n";
	for (auto t : Iter::From(a).Chain(Iter::From(a))) {
		std::cout << t << " ";
	}

	std::cout << "\n\nIter::FromRef(a).Enumerate() => rand() => Iter::FromRef(a).Enumerate():\n";
	for (auto [i, v] : Iter::FromRef(a).Enumerate()) {
		std::cout << i << ": " << v << "\n";
		v = rand();
		std::cout << i << ": " << v << "\n";
	}

	std::cout << "\nIter::From(a):\n";
	for (auto v : Iter::From(a)) {
		std::cout << v << " ";
	}

	auto half_a = Iter::From(a).Take(5).ToList();
	std::cout << "\n\nHalf of a as list: count = "
		<< Iter::From(half_a).Count() << "; ref count = " << Iter::FromRef(half_a).Count() << ";\n";
	for (auto t : Iter::From(half_a)) {
		std::cout << t << " ";
	}

	std::cout << "\n\nIter::FwdRange(1, 6).Product() => 5! = " << Iter::FwdRange(1, 6).Product() << "\n";
	std::cout << "Iter::FwdRange(0, 10).Map(x => x*x).Enumerate():\n";
	for (auto t : Iter::FwdRange(0, 10).Map([](auto x) { return x * x; }).Enumerate()) {
		std::cout << t << " ";
	}

	std::cout << "\n\nReverse range [0; 10):\n";
	for (auto v : Iter::FwdRange(0, 10).Reverse()) {
		std::cout << v << " ";
	}

	{
		std::cout << "\n";
		auto it = Iter::DDRange(0, 5);
		for (auto v : it) {
			std::cout << v << " ";
		}
	}
	{
		std::cout << "\n";
		auto it = Iter::DDRange(0, 5);
		for (auto v : it) {
			std::cout << v << " ";
		}
	}
	{
		std::cout << "\n";
		auto it = Iter::DDRange(0, 5000000).Reverse().Take(5);
		for (auto v : it.Reverse()) {
			std::cout << v << " ";
		}
	}
}
