# CppIterators
### Factorial example

```cpp
int64_t factorial(int64_t x) {
	return Iter::Range(int64_t(1), x).Product();
}

int main() {
	for (auto [i, x] : Iter::Range(2).Map(factorial).Take(10).Enumerate(1))
		std::cout << i << "! =\t" << x << "\n";
}
```
The code prints:
```
1! =    1
2! =    2
3! =    6
4! =    24
5! =    120
6! =    720
7! =    5040
8! =    40320
9! =    362880
10! =   3628800
```
