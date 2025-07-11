#pragma once
#include <cstddef>
#include <emscripten/fetch.h>
#include <string>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <functional>
#include "augs/log.h"

namespace augs {
    class emscripten_http {
        std::string base_url_;
        int io_timeout = 5;

    public:
        struct response {
            int status = -1;
            std::string body;
        };

        emscripten_http(const std::string& base_url, const int io_timeout) : base_url_(base_url), io_timeout(io_timeout) {}

        using result = std::unique_ptr<response>;
        using Headers = std::unordered_map<std::string, std::string>;

#if 0
		/* dont care about this for now */
        result Post(const std::string& location, const Headers& headers, const char* body, size_t content_length, const std::string& content_type) {
            emscripten_fetch_attr_t attr;
            emscripten_fetch_attr_init(&attr);

            strcpy(attr.requestMethod, "POST");
            attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS;
            attr.timeoutMSecs = io_timeout * 1000;

            // Headers handling
            std::vector<const char*> headerData;
            for (const auto& header : headers) {
                headerData.push_back(header.first.c_str());
                headerData.push_back(header.second.c_str());
            }
            headerData.push_back(nullptr); // Terminate the headers array
            attr.requestHeaders = headerData.data();

            // Setup the POST body data
            attr.requestData = body;
            attr.requestDataSize = content_length;
            attr.overriddenMimeType = content_type.c_str();

            std::string full_url = base_url_ + location;

            response resp;

            if (emscripten_fetch_t* fetch = emscripten_fetch(&attr, full_url.c_str())) {
                resp.status = fetch->status;

                if (fetch->status == 200) {
                    resp.body.assign(reinterpret_cast<const char*>(fetch->data), fetch->numBytes);
                }

                emscripten_fetch_close(fetch);
            }

            return std::make_unique<response>(std::move(resp));
        }
#endif

        result Get(const std::string& location, std::function<void(std::size_t, std::size_t)> on_progress = nullptr, const Headers& headers = {}) {
			(void)on_progress;

            emscripten_fetch_attr_t attr;
            emscripten_fetch_attr_init(&attr);

            strcpy(attr.requestMethod, "GET");
            attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS;
            attr.timeoutMSecs = io_timeout * 1000;

            // Headers handling
            std::vector<const char*> headerData;
            for (const auto& header : headers) {
                headerData.push_back(header.first.c_str());
                headerData.push_back(header.second.c_str());
            }

            headerData.push_back(nullptr); // Terminate the headers array
            attr.requestHeaders = headerData.data();

            std::string full_url = base_url_ + location;

			LOG("GET %x", full_url);

            response resp;

            if (emscripten_fetch_t* fetch = emscripten_fetch(&attr, full_url.c_str())) {
                resp.status = fetch->status;

                LOG_NVPS(fetch->status);

                if (fetch->status == 200) {
                    resp.body.assign(reinterpret_cast<const char*>(fetch->data), fetch->numBytes);
                }

                emscripten_fetch_close(fetch);
            }
			else {
				LOG("Fetch result was null.");
			}

            return std::make_unique<response>(std::move(resp));
        }

		result Get(const std::string& location, const Headers& headers, std::function<void(std::size_t, std::size_t)> on_progress = nullptr) {
			return Get(location, on_progress, headers);
		}

        void set_keep_alive(bool) {

        }
    };
}
