#pragma once
#include <memory>
#include "augs/misc/enum/enum_boolset.h"

#if BUILD_FREETYPE
struct FT_LibraryRec_;
typedef struct FT_LibraryRec_  *FT_Library;
#endif

namespace augs {
	struct global_libraries {
		enum class library {
			FREETYPE,
			NETWORKING,

			COUNT
		};

		using library_flagset = augs::enum_boolset<library>;

		library_flagset initialized;
#if BUILD_FREETYPE
		static std::unique_ptr<FT_Library> freetype_library;
#endif

		global_libraries(library_flagset);
		
		global_libraries(const global_libraries&) = delete;
		global_libraries& operator=(const global_libraries&) = delete;

		global_libraries(global_libraries&&) = delete;
		global_libraries& operator=(global_libraries&&) = delete;
		
		~global_libraries();

		void init		(library_flagset = { library::FREETYPE, library::NETWORKING });
		void deinit		(library_flagset);
	};

	struct network_raii {
		network_raii();
		~network_raii();

		network_raii(const network_raii&) = delete;
		network_raii(network_raii&&) = delete;

		network_raii& operator=(const network_raii&) = delete;
		network_raii& operator=(network_raii&&) = delete;
	};
};