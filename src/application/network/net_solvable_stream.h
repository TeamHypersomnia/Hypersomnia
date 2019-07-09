#pragma once

template <class V>
constexpr bool never_changes_in_game = is_one_of_v<V,
	make_entity_pool<sprite_decoration>,
	make_entity_pool<box_marker>,
	make_entity_pool<particles_decoration>,
	make_entity_pool<wandering_pixels_decoration>,
	make_entity_pool<point_marker>,
	make_entity_pool<static_light>
>;

using physics_bodies = make_entity_pool<plain_sprited_body>;
using physics_bodies_vector = typename physics_bodies::object_pool_type;

using complex_decorations = make_entity_pool<complex_decoration>;
using complex_decorations_vector = typename complex_decorations::object_pool_type;

struct net_solvable_stream_ref : augs::ref_memory_stream {
	using base = augs::ref_memory_stream;

	const all_entity_flavours& flavours;
	const cosmos_solvable_significant& initial_signi;
	const cosmos_solvable_significant& serialized_signi;

	template <class... Args>
	net_solvable_stream_ref(
		const all_entity_flavours& flavours,
		const cosmos_solvable_significant& initial_signi,
		const cosmos_solvable_significant& serialized_signi,
		Args&&... args
	) : 
		base(std::forward<Args>(args)...),
		flavours(flavours),
		initial_signi(initial_signi),
		serialized_signi(serialized_signi)
	{}

	template <class T, class = std::enable_if_t<never_changes_in_game<T>>>
	void special_write(const T& storage) {
		(void)storage;
	}

	template <class V, class Pred>
	void special_write_static_or_not(const V& storage, Pred pred) {
		using E = entity_type_of<typename V::value_type>;

		const auto& body_flavours = flavours.template get_for<E>();
		const auto& serialized_pool = serialized_signi.entity_pools.get_for<E>();
		const auto& original_pool = initial_signi.entity_pools.get_for<E>();

		augs::write_bytes(*this, storage.size());

		for (const auto& s : storage) {
			const bool is_always_static = pred(body_flavours[s.flavour_id]);
			const auto this_idx = index_in(storage, s);
			const auto this_id = serialized_pool.find_nth_id(this_idx);

			const auto static_byte = [&]() -> char {
				if (is_always_static) {
					return 1;
				}

				static_assert(std::is_trivially_copyable_v<remove_cref<decltype(s)>>);

				if (const auto correspondent_initial = original_pool.find(this_id)) {
					if (!std::memcmp(std::addressof(s), correspondent_initial, sizeof(*correspondent_initial))) {
						return 2;
					}
				}

				return 0;
			}();
			 
			augs::write_bytes(*this, static_byte);
			
			if (static_byte == 0) {
				augs::write_bytes(*this, s);
			}
			else {
				augs::write_bytes(*this, this_id.to_unversioned());
			}
		}
	}

	void special_write(const physics_bodies_vector& storage) {
		special_write_static_or_not(storage, [&](const auto& flav) {
			return flav.template get<invariants::rigid_body>().body_type == rigid_body_type::ALWAYS_STATIC;
		});
	}

	void special_write(const complex_decorations_vector& storage) {
		special_write_static_or_not(storage, [&](const auto& flav) {
			return flav.template get<invariants::animation>().is_irrelevant_to_logic;
		});
	}
};

struct net_solvable_stream_cref : augs::cref_memory_stream {
	using base = augs::cref_memory_stream;

	const cosmos_solvable_significant& initial_signi;

	template <class... Args>
	net_solvable_stream_cref(
		const cosmos_solvable_significant& initial_signi, 
		Args&&... args
	) : 
		base(std::forward<Args>(args)...),
		initial_signi(initial_signi)
	{}

	template <class T, class = std::enable_if_t<never_changes_in_game<T>>>
	void special_read(T& storage) {
		storage = initial_signi.entity_pools.get<T>();
	}

	template <class V>
	void special_read_static_or_not(V& storage) {
		using E = entity_type_of<typename V::value_type>;
		const auto& initial_bodies = initial_signi.entity_pools.get_for<E>();

		using size_type = decltype(storage.size());

		size_type n;
		augs::read_bytes(*this, n);

		resize_no_init(storage, n);

		using unversioned_id_type = typename remove_cref<decltype(initial_bodies)>::unversioned_id_type;

		for (size_type i = 0; i < n; ++i) {
			char static_byte;
			augs::read_bytes(*this, static_byte);

			if (static_byte == 0) {
				augs::read_bytes(*this, storage[i]);
			}
			else {
				unversioned_id_type id;
				augs::read_bytes(*this, id);

				storage[i] = initial_bodies[initial_bodies.get_versioned(id)];
			}
		}
	}

	void special_read(physics_bodies_vector& storage) {
		special_read_static_or_not(storage);
	}

	void special_read(complex_decorations_vector& storage) {
		special_read_static_or_not(storage);
	}
};

static_assert(augs::has_special_read_v<net_solvable_stream_cref, complex_decorations_vector>);
