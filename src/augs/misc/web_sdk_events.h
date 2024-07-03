#pragma once
#include "augs/misc/scope_guard.h"

void web_sdk_loading_start();
void web_sdk_loading_stop();

struct web_sdk_loading_raii {
	web_sdk_loading_raii() {
		web_sdk_loading_start();
	}

	~web_sdk_loading_raii() {
		web_sdk_loading_stop();
	}

	web_sdk_loading_raii(const web_sdk_loading_raii&) = delete;
	web_sdk_loading_raii(web_sdk_loading_raii&&) = delete;

	web_sdk_loading_raii& operator=(const web_sdk_loading_raii&) = delete;
	web_sdk_loading_raii& operator=(web_sdk_loading_raii&&) = delete;
};
