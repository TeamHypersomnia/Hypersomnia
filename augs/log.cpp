#include "augs/log.h"

#include <iostream>
#include <string>
#include <gtest/gtest.h>

#include "augs/math/vec2.h"


#include <fstream>
#include <thread>
#include <mutex>

std::mutex log_mutex;

template<>
void LOG(std::string f) {
	std::unique_lock<std::mutex> lock(log_mutex);

	std::cout << f << std::endl;
	std::ofstream recording_file("live_debug.txt", std::ios::out | std::ios::app);
	recording_file << f << std::endl;
}

template<>
void LOG_COLOR(console_color c, std::string f) {
	std::unique_lock<std::mutex> lock(log_mutex);

	augs::colored_print(c, f.c_str());
	std::ofstream recording_file("live_debug.txt", std::ios::out | std::ios::app);
	recording_file << f << std::endl;
}

void CALL_SHELL(std::string s) {
	std::unique_lock<std::mutex> lock(log_mutex);

	system(s.c_str());
}

TEST(TypesafeSprintf, TypesafeSprintfSeveralTests) {
	EXPECT_EQ("1,2,3:4", typesafe_sprintf("%x,%x,%x:%x", 1, 2, 3, 4));
	EXPECT_EQ("abc,2,3:def", typesafe_sprintf("%x,%x,%x:%x", "abc", 2, 3, "def"));
	EXPECT_EQ("abc,2.55,3.14:def", typesafe_sprintf("%x,%x,%x:%x", "abc", 2.55, 3.14f, "def"));

	vec2 test(123, 412);

	EXPECT_EQ("Vector is equal to: (123,412)", typesafe_sprintf("Vector is equal to: %x", test));

	int errid = 1282;
	std::string location = "augs::window::glwindow::create";

	EXPECT_EQ("OpenGL error 1282 in augs::window::glwindow::create", typesafe_sprintf("OpenGL error %x in %x", errid, location));
}