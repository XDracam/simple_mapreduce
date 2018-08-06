#include "stdafx.h"
#include "simple_map_reduce.h"
#include <iterator>
#include <iostream>
#if _WIN32
#include <conio.h>
#endif
#include <numeric>
#include <algorithm>
#include <array>
#include <vector>
#include <forward_list>

constexpr int double_int(int a) { return a * 2; }
constexpr int add_ints(int a, int b) { return a + b; }

using namespace std;
using namespace smr;

int main(const int argc, const char*const argv[])
{
	std::vector<string> args;
	const vector<int> numbers{ 1,2,3,4,5 };
	const array<int, 5> number_array = { 1,2,3,4,5 };
	const list<int> number_list{ 1,2,3,4,5 };
	const forward_list<int> number_flist{ 1,2,3,4,5 };
	const list<vector<int>> nested{ vector<int>{1,2,3}, vector<int>{4,5,6} };
	const vector<string> lels{ "lel", "lel", "lel", "lel" };

	// look, terrible boilerplate since it's so generic
	auto x = accumulate(numbers.begin(), numbers.end(), 0, [](auto a, auto b) {return a + b; });
	cout << "Sum of numbers is " << x << endl;
	// but accumulate is just a fold, and a fold with + is a sum
	// introducing new lambda syntax for convenience
	cout << "Sum of numbers with foldl is " << foldl(0, numbers, λ2(_1 + _2)) << endl;
	cout << "Sum of numbers from array is " << foldl(0, number_array, λn(_ + ...)) << endl; // fold expression
	cout << "A fold with + is just a sum: " << sum(numbers) << " ; " << sum(lels) << endl;

	// what in insane amount of boilerplate for a simple fmap
	list<int> results{};
	transform(number_list.begin(), number_list.end(), back_inserter(results), double_int);

	// introduce tostring for arbitrary containers of printables, together with fmap and filter
	cout << "Numbers times two is " << show(fmap<list>(number_list, double_int)) << endl;
	cout << "You can filter all even numbers, or double all odd ones: "
		<< show(filter(numbers, λ(_ % 2 == 0))) << " ; "
		<< show(collect<vector>(numbers, double_int, λ(_ % 2 != 0))) << endl;

	// monadic properties of any sequence may be used as well
	cout << "Flatmapping numbers to identity, square and cube: " <<
		// standard lambda syntax pretty boilerplatey here
		show(flatmap<vector>(numbers, [](int a) {return forward_list<int>{ a, a*a, a*a*a }; })) << endl;
	cout << "Flattening works fine as well: " <<
		show(nested, λ(show(_))) <<
		" -> " << show(flatten<list>(nested)) << endl;

	// provide fill and tabulate to construct new sequences easily
	cout << "Lets fill a list with 'lel's: " <<
		// _ret(foo) is shorthand for []{return foo;}, constant generator (necessary?)
		show(fill<list>(5, _ret("lel"))) << endl;
	cout << "Or tabulate square numbers: " <<
		show(tabulate<vector>(5, [](int a) -> int {return a * a; })) << endl;


	// offer versions of every higher order function with compiletime functions and forced inlining
	// providing so many types is necessary, since I don't have enough experience to make inference just work
	cout << "Inline mapreduce functions with compiletime template functions:" << endl;
	cout << foldl<vector<int>, int, [](int a, int b) {return a + b; }>(0, numbers) << " ; ";
	cout << show(fmap<vector, int, int, double_int>(numbers)) << endl;
	cout << show(flatmap<vector, forward_list<int>, int, λ(forward_list<int>({ _, _*_, _*_*_ }))>(numbers)) << endl;
	//cout << show(fill<list, string, []{return "woot"; }>(5)) << endl;
	cout << show(collect<list, int, string, λ("#" + to_string(_)), λ(_ > 1)>(number_list)) << endl;

	// provide utilities to construct maps and and handle them more easily
	cout << "Maps are necessary when dealing efficiently with associations:" << endl;
	cout << show(to_map(numbers, λ("#" + to_string(_)))) << endl;
	cout << show(to_map<list, int, string, λ("#" + to_string(_ - 1))>(number_list)) << endl;
	cout << show(group_by<vector>(tabulate<vector>(10, identity), λ(_ % 3)), λ(to_str(_))) << endl;
	auto const grouped = group_by<list>(tabulate<list>(7, λ(_ / 2)), identity);
	cout << show(fmap_values<map>(grouped, λ(show(_)))) << endl;
	cout << show(fmap_values<
			map, decltype(grouped)::key_type, 
			decltype(grouped)::mapped_type, 
			string, λ(show(_))
		>(grouped)) << endl;
	cout << "Keys: " << keys<map>(grouped) << "; Values: "
		<< fmap<vector>(values<map>(grouped), λ(show(_))) << endl;

	// sometimes, grouped or sliding traversal is useful. These are inefficient for larger sequences.
	cout << "Grouped copy view: " << show(grouped_copy<2, vector>(numbers), λ(show(_))) << endl;
	cout << "Sliding copy view: " << show(sliding_copy<3, list>(number_list), λ(show(_))) << endl;

#if _WIN32
	_getch();
#endif

	return 0;
}