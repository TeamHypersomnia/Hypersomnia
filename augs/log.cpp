#include "augs/log.h"

#include <iostream>
#include <string>
#include <gtest/gtest.h>

#include "augs/math/vec2.h"

#include <fstream>
#include <thread>
#include <mutex>

#include "augs/filesystem/file.h"
#include "augs/templates/string_templates.h"

#define ENABLE_LOG 1
#define LOG_TO_FILE 0

std::mutex log_mutex;

unsigned global_log::max_all_entries = 10000;
std::vector<log_entry> global_log::all_entries;

augs::gui::text::formatted_string global_log::format_recent_as_text(
	const assets::font_id f,
	unsigned lines_remaining
) {
	augs::gui::text::formatted_string result;
	
	lines_remaining = std::min(lines_remaining, all_entries.size());

	for (auto it = all_entries.end() - lines_remaining; it != all_entries.end(); ++it) {
		std::stringstream ss((*it).text);
		std::string line;

		auto wstr = to_wstring((*it).text + "\n");
		result += augs::gui::text::format(wstr, augs::gui::text::style(f, rgba((*it).color)));

		--lines_remaining;
	}

	return result;
}

void global_log::push_entry(const log_entry& new_entry) {
	all_entries.push_back(new_entry);

	if (all_entries.size() > max_all_entries) {
		all_entries.erase(all_entries.begin(), all_entries.begin() + max_all_entries/5);
	}
}

void global_log::save_complete_log(const std::string& filename) {
	std::string complete_log;

	for (const auto& e : all_entries) {
		complete_log += e.text + '\n';
	}
	
	augs::create_text_file(filename, complete_log);
}

template<>
void LOG(const std::string& f) {
#if ENABLE_LOG 
	std::unique_lock<std::mutex> lock(log_mutex);

	global_log::push_entry({ console_color::WHITE, f });

	std::cout << f << std::endl;
#if LOG_TO_FILE
	std::ofstream recording_file("generated/logs/live_debug.txt", std::ios::out | std::ios::app);
	recording_file << f << std::endl;
#endif
#endif
}

template<>
void LOG_COLOR(const console_color c, const std::string& f) {
#if ENABLE_LOG 
	std::unique_lock<std::mutex> lock(log_mutex);
	
	global_log::push_entry({ c, f });

	augs::colored_print(c, f.c_str());
#if LOG_TO_FILE
	std::ofstream recording_file("generated/logs/live_debug.txt", std::ios::out | std::ios::app);
	recording_file << f << std::endl;
#endif
#endif
}


void CALL_SHELL(const std::string& s) {
	std::unique_lock<std::mutex> lock(log_mutex);

	system(s.c_str());
}

TEST(TypesafeSprintf, TypesafeSprintfSeveralTests) {
	EXPECT_EQ("1,2,3:4", typesafe_sprintf("%x,%x,%x:%x", 1, 2, 3, 4));
	EXPECT_EQ("abc,2,3:def", typesafe_sprintf("%x,%x,%x:%x", "abc", 2, 3, "def"));
	EXPECT_EQ("abc,2.55,3.14:def", typesafe_sprintf("%x,%x,%x:%x", "abc", 2.55, 3.14f, "def"));

	vec2 test(123, 412);

	EXPECT_EQ("Vector is equal to: (123;412)", typesafe_sprintf("Vector is equal to: %x", test));

	int errid = 1282;
	std::string location = "augs::window::glwindow::create";

	EXPECT_EQ("OpenGL error 1282 in augs::window::glwindow::create", typesafe_sprintf("OpenGL error %x in %x", errid, location));

	int a = 2;
	LOG_NVPS("Test nvps: ", a, errid, test);
}

TEST(TypesafeSscanf, TypesafeSscanfSeveralTests) {
	{
		const auto format = "%x";
		const auto sprintfed = "1442";

		unsigned s1 = 0xdeadbeef;
		typesafe_sscanf(sprintfed, format, s1);

		EXPECT_EQ(1442, s1);
	}

	{
		const auto format = "%x,%x";
		const auto sprintfed = "1442,1337";

		unsigned s1 = 0xdeadbeef;
		unsigned s2 = 0xdeadbeef;
		typesafe_sscanf(sprintfed, format, s1, s2);

		EXPECT_EQ(1442, s1);
		EXPECT_EQ(1337, s2);
	}

	{
		const auto format = "%x,%x,%x:%x";
		const auto sprintfed = typesafe_sprintf(format, 1, 2, 3, 4);
		EXPECT_EQ("1,2,3:4", sprintfed);

		int s1, s2, s3, s4;
		typesafe_sscanf(sprintfed, format, s1, s2, s3, s4);

		EXPECT_EQ(1, s1);
		EXPECT_EQ(2, s2);
		EXPECT_EQ(3, s3);
		EXPECT_EQ(4, s4);
	}

	{


		vec2i test(123, -412);
		const auto format = "Vector is equal to: %x";
		const auto sprintfed = typesafe_sprintf(format, test);
		EXPECT_EQ("Vector is equal to: (123;-412)", sprintfed);

		vec2i read_test;

		typesafe_sscanf(sprintfed, format, read_test);

		EXPECT_EQ(test, read_test);
	}
}

#include "augs/misc/enum_bitset.h"

TEST(CustomContainers, EnumBitsetTest) {
	enum class aa {
		A,
		B,

		COUNT
	};

	{
		augs::enum_bitset<aa> myar{ aa::B };

		EXPECT_TRUE(myar.test(aa::B));
		EXPECT_FALSE(myar.test(aa::A));
	}

	{
		augs::enum_bitset<aa> myar2{ aa::A, aa::B };

		EXPECT_TRUE(myar2.test(aa::A));
		EXPECT_TRUE(myar2.test(aa::B));
	}

	{
		augs::enum_bitset<aa> myar3{ aa::B, aa::A };

		EXPECT_TRUE(myar3.test(aa::B));
		EXPECT_TRUE(myar3.test(aa::A));
	}
}