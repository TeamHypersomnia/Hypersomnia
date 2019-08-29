

template <class T>
struct is_constant_size_string : std::false_type {};

template <std::size_t N>
struct is_constant_size_string<augs::constant_size_string<N>> : std::true_type {};

template <class T>
static constexpr bool is_constant_size_string_v = is_constant_size_string<T>::value; 