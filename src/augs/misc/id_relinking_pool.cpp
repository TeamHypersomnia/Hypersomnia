#if BUILD_UNIT_TESTS
#include <catch.hpp>
#include "augs/misc/id_relinking_pool.h"
#include "augs/misc/constant_size_vector.h"

using p_t = augs::id_relinking_pool<int, of_size<6>::make_constant_vector>;
using k_t = p_t::key_type; 

struct test_allocation {
	k_t key;
	int value = -1;

	bool operator==(const test_allocation& b) const {
		return key == b.key && value == b.value;
	}

	test_allocation() = default;

	test_allocation(const p_t::allocation_result r) {
		key = r.key;
		value = r.object;
	}

	bool consistent(const p_t& p) const {
		return p.get(key) == value;
	}

	void require_dead(const p_t& p) const {
		REQUIRE(key.operator bool() == false);

		const auto found = p.find(key);
		REQUIRE(found == nullptr);
	}

	void require_exists_and_consistent(const p_t& p) const {
		const auto found = p.find(key);

		REQUIRE(key.operator bool());
		REQUIRE(found != nullptr);

		REQUIRE(*found == value);
	}

	void require_consistent(const p_t& p) const {
		if (key) {
			REQUIRE(p.get(key) == value);
		}
	}
};

std::ostream& operator<<(std::ostream& out, const test_allocation& x) {
	return out << x.key << " = " << x.value;
}

using kv_t = std::vector<test_allocation>;

#define TEST_ALLOCATIONS 6

auto make_for_each_id(kv_t& v) {
	return [&v](auto callback) {
		for (auto& k : v) {
			callback(k.key);
		}
	};
}

TEST_CASE("IdRelinkingPool SimpleBackAndForth") {
	p_t p;
	kv_t allocations;
	allocations.resize(TEST_ALLOCATIONS);

	auto for_each_id = make_for_each_id(allocations);

	for (int trial = 0; trial < 3; ++trial) {
		REQUIRE(p.empty());

		for (unsigned i = 0; i < TEST_ALLOCATIONS; ++i) {
			allocations[i] = p.allocate(for_each_id, i);
			allocations[i].require_exists_and_consistent(p);
		}

		REQUIRE(p.full());

		for (unsigned i = 0; i < TEST_ALLOCATIONS; ++i) {
			allocations[i].require_exists_and_consistent(p);
		}

		for (unsigned i = 0; i < TEST_ALLOCATIONS; ++i) {
			p.free(for_each_id, allocations[i].key);
			allocations[i].require_dead(p);
		}

		REQUIRE(p.empty());
	}
}

TEST_CASE("IdRelinkingPool UndoAllocations") {
	p_t p;

	kv_t allocations;
	allocations.resize(TEST_ALLOCATIONS);

	kv_t allocations_after;
	allocations_after.resize(TEST_ALLOCATIONS);

	auto for_each_id = make_for_each_id(allocations);
	auto for_each_after_id = make_for_each_id(allocations_after);

	for (int trial = 0; trial < 3; ++trial) {
		REQUIRE(p.empty());

		for (unsigned i = 0; i < TEST_ALLOCATIONS; ++i) {
			const auto new_alloc = p.allocate(for_each_id, i);
			allocations[i] = new_alloc;
			allocations[i].require_exists_and_consistent(p);
		}

		{
			const auto allocations_backup = allocations;

			for (int idx = TEST_ALLOCATIONS - 1; idx >= 0; --idx) {
				const auto i = static_cast<unsigned>(idx);

				allocations[i].require_exists_and_consistent(p);
				p.undo_last_allocate(for_each_id);
				allocations[i].require_dead(p);
			}

			for (unsigned i = 0; i < TEST_ALLOCATIONS; ++i) {
				allocations_after[i] = p.allocate(for_each_after_id, i);
			}

			REQUIRE(allocations_after == allocations_backup);
		}

		REQUIRE(p.full());

		/* Cleanup */
		for (int idx = static_cast<int>(TEST_ALLOCATIONS) - 1; idx >= 0; --idx) {
			auto i = static_cast<unsigned>(idx);

			allocations_after[i].require_exists_and_consistent(p);
			p.undo_last_allocate(for_each_id);
		}
	}
}

TEST_CASE("IdRelinkingPool UndoDeletes") {
	p_t p;

	kv_t allocations;
	allocations.resize(TEST_ALLOCATIONS);

	std::vector<k_t> undo_free_inputs;
	undo_free_inputs.resize(TEST_ALLOCATIONS);

	auto for_each_id = make_for_each_id(allocations);

	for (int trial = 0; trial < 3; ++trial) {
		REQUIRE(p.empty());

		for (unsigned i = 0; i < TEST_ALLOCATIONS; ++i) {
			allocations[i] = p.allocate(for_each_id, i);
			allocations[i].require_exists_and_consistent(p);
		}

		REQUIRE(p.full());

		auto undo_free = [&p, &allocations, &for_each_id, &undo_free_inputs](const auto i) {
			allocations[i].require_dead(p);
			p.undo_free(for_each_id, undo_free_inputs[i], allocations[i].value);

			LOG("Unfree: %x, undid id: %x", i, undo_free_inputs[i]);

			for (const auto& a : allocations) {
				LOG_NVPS(a);
			}

			allocations[i].require_exists_and_consistent(p);
		};

		auto do_free = [&p, &allocations, &for_each_id, &undo_free_inputs](const auto i) {
			allocations[i].require_exists_and_consistent(p);

			const auto key = allocations[i].key;
			undo_free_inputs[i] = key; 

			p.free(for_each_id, key);
			allocations[i].require_dead(p);
		};

		// forward direction frees
		{
			const auto allocations_backup = allocations;

			for (unsigned i = 0; i < TEST_ALLOCATIONS; ++i) {
				do_free(i);

				LOG("Free: %x, undid id: %x", i, undo_free_inputs[i]);

				for (const auto& a : allocations) {
					LOG_NVPS(a);
				}
			}

			REQUIRE(p.empty());

			for (int idx = static_cast<int>(TEST_ALLOCATIONS) - 1; idx >= 0; --idx) {
				const auto i = static_cast<unsigned>(idx);

				undo_free(i);
			}

			REQUIRE(allocations_backup == allocations);
		}

		// reverse direction frees
		{
			const auto allocations_backup = allocations;

			for (int idx = static_cast<int>(TEST_ALLOCATIONS) - 1; idx >= 0; --idx) {
				const auto i = static_cast<unsigned>(idx);

				do_free(i);
			}

			REQUIRE(p.empty());

			for (unsigned i = 0; i < TEST_ALLOCATIONS; ++i) {
				undo_free(i);
			}

			REQUIRE(allocations_backup == allocations);
		}

		/* cleanup */
		for (unsigned i = 0; i < TEST_ALLOCATIONS; ++i) {
			allocations[i].require_exists_and_consistent(p);
			p.free(for_each_id, allocations[i].key);
		}
	}
}

#if 0
template <class T>
void test_pool() {
	kv_t keys;
	keys.resize(4);

	auto fe = make_for_each_id(keys);

	T p;

	readwrite_test_cycle(p);
	p.allocate(fe, 1);
	readwrite_test_cycle(p);
	p.allocate(fe, 2);
	readwrite_test_cycle(p);
	p.allocate(fe, 33);
	readwrite_test_cycle(p);

	keys[0] = p.allocate(fe, 1);
	readwrite_test_cycle(p);
	keys[1]= p.allocate(fe, 3);
	readwrite_test_cycle(p);
	keys[2]= p.allocate(fe, 3);
	readwrite_test_cycle(p);
	keys[3]= p.allocate(fe, 3);
	readwrite_test_cycle(p);
	p.free(fe, keys[1]);
	readwrite_test_cycle(p);
	p.free(fe, keys[3]);
	readwrite_test_cycle(p);

	REQUIRE(p.find(keys[3]) == nullptr);
	REQUIRE(p.find(keys[1]) == nullptr);
	REQUIRE(p.find(keys[0]) != nullptr);
	REQUIRE(p.find(keys[2]) != nullptr);

	REQUIRE(5 == p.size());
}

TEST_CASE("IdRelinkingPool Readwrite") {
	test_pool<augs::id_relinking_pool<float, of_size<100>::make_constant_vector>>();
	test_pool<augs::id_relinking_pool<float, make_vector>>();
}
#endif

#endif
