#pragma once
#define USE_NAMES_FOR_IDS

#ifdef USE_NAMES_FOR_IDS
#include <string>
#endif

namespace augs {
	class raw_pool_id {
	public:
#ifdef USE_NAMES_FOR_IDS
		char debug_name[40];
#endif
		int version = 0xdeadbeef;
		int indirection_index = -1;

		raw_pool_id();

		void unset();

		void set_debug_name(std::string s);

		std::string get_debug_name() const;

		bool operator==(const raw_pool_id& b) const;
		bool operator!=(const raw_pool_id& b) const;
		bool operator<(const raw_pool_id& b) const;

		friend std::ostream& operator<<(std::ostream& out, const raw_pool_id &x);

		template <class Archive>
		void serialize(Archive& ar) {
#ifdef USE_NAMES_FOR_IDS
			ar(CEREAL_NVP(debug_name));
#endif
			ar(CEREAL_NVP(version), CEREAL_NVP(indirection_index));
		}
	};

	template<class owner_pool_type>
	class pool_id : public raw_pool_id {
	public:
		using raw_pool_id::raw_pool_id;
		using raw_pool_id::operator==;
		using raw_pool_id::operator!=;
		using raw_pool_id::operator<;
	};

	template<class T>
	struct make_pool_id { typedef pool_id<T> type; };
}