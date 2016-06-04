#pragma once
#include <vector>
#include <algorithm>

template <class T, class Tuple>
struct index_in_tuple;

template <class T, class... Types>
struct index_in_tuple<T, std::tuple<T, Types...>> {
	static const std::size_t value = 0;
};

template <class T, class U, class... Types>
struct index_in_tuple<T, std::tuple<U, Types...>> {
	static const std::size_t value = 1 + index_in_tuple<T, std::tuple<Types...>>::value;
};

template<class T, class L>
void erase_remove(std::vector<T>& v, const L& l) {
	v.erase(std::remove_if(v.begin(), v.end(), l), v.end());
}

template<class It> It uniquify(It begin, It const end)
{
	struct target_less
	{
		template<class It>
		bool operator()(It const &a, It const &b) const { return *a < *b; }
	};

	struct target_equal
	{
		template<class It>
		bool operator()(It const &a, It const &b) const { return *a == *b; }
	};

    std::vector<It> v;
    v.reserve(static_cast<size_t>(std::distance(begin, end)));
    for (It i = begin; i != end; ++i)
    { v.push_back(i); }
    std::sort(v.begin(), v.end(), target_less());
    v.erase(std::unique(v.begin(), v.end(), target_equal()), v.end());
    std::sort(v.begin(), v.end());
    size_t j = 0;
    for (It i = begin; i != end && j != v.size(); ++i)
    {
        if (i == v[j])
        {
            using std::iter_swap; iter_swap(i, begin);
            ++j;
            ++begin;
        }
    }
    return begin;
}