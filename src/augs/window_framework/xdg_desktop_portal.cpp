#include <atomic>
#include <cstdint>
#include <fcntl.h>
#include <unistd.h>
#include <dbus/dbus.h>
#include "augs/log.h"
#include "augs/window_framework/xdg_desktop_portal.h"

namespace augs {
	namespace xdg_desktop_portal {
		constexpr const char* bus_name = "org.freedesktop.portal.Desktop";
		constexpr const char* object_path = "/org/freedesktop/portal/desktop";
		constexpr const char* file_chooser_interface = "org.freedesktop.portal.FileChooser";
		constexpr const char* open_uri_interface = "org.freedesktop.portal.OpenURI";
		constexpr const char* request_interface = "org.freedesktop.portal.Request";

		constexpr int activation_timeout_ms = 25000;
		constexpr int version_query_timeout_ms = 3000;
		constexpr int response_poll_interval_ms = 100;
		constexpr std::uint32_t first_version_opening_directories = 3;
		constexpr std::uint32_t response_success = 0;

		static void ensure_threads_initialized() {
			static const bool initialized = dbus_threads_init_default();
			(void)initialized;
		}

		class session_bus {
			DBusConnection* connection = nullptr;

		public:
			session_bus() {
				ensure_threads_initialized();

				DBusError error;
				dbus_error_init(&error);

				connection = dbus_bus_get_private(DBUS_BUS_SESSION, &error);

				if (dbus_error_is_set(&error)) {
					LOG("Desktop portal: no session bus (%x: %x).", error.name, error.message);
					dbus_error_free(&error);
				}

				if (connection != nullptr) {
					dbus_connection_set_exit_on_disconnect(connection, false);
				}
			}

			~session_bus() {
				if (connection != nullptr) {
					dbus_connection_close(connection);
					dbus_connection_unref(connection);
				}
			}

			session_bus(const session_bus&) = delete;
			session_bus& operator=(const session_bus&) = delete;

			DBusConnection* get() const {
				return connection;
			}
		};

		class message_ptr {
			DBusMessage* message = nullptr;

		public:
			explicit message_ptr(DBusMessage* const message) : message(message) {}

			~message_ptr() {
				if (message != nullptr) {
					dbus_message_unref(message);
				}
			}

			message_ptr(const message_ptr&) = delete;
			message_ptr& operator=(const message_ptr&) = delete;

			DBusMessage* get() const {
				return message;
			}
		};

		static message_ptr call_and_wait_for_reply(
			DBusConnection* const connection,
			DBusMessage* const call,
			const char* const what,
			const int timeout_ms = activation_timeout_ms
		) {
			DBusError error;
			dbus_error_init(&error);

			DBusMessage* const reply = dbus_connection_send_with_reply_and_block(connection, call, timeout_ms, &error);

			if (dbus_error_is_set(&error)) {
				LOG("Desktop portal: %x failed (%x: %x).", what, error.name, error.message);
				dbus_error_free(&error);
			}

			return message_ptr(reply);
		}

		static std::optional<std::uint32_t> get_interface_version(DBusConnection* const connection, const char* const interface_name) {
			const auto call = message_ptr(dbus_message_new_method_call(bus_name, object_path, "org.freedesktop.DBus.Properties", "Get"));
			const char* const property_name = "version";

			dbus_message_append_args(
				call.get(),
				DBUS_TYPE_STRING, &interface_name,
				DBUS_TYPE_STRING, &property_name,
				DBUS_TYPE_INVALID
			);

			const auto reply = call_and_wait_for_reply(connection, call.get(), "getting the interface version", version_query_timeout_ms);

			if (reply.get() == nullptr) {
				return std::nullopt;
			}

			DBusMessageIter reply_iter;

			if (!dbus_message_iter_init(reply.get(), &reply_iter) || dbus_message_iter_get_arg_type(&reply_iter) != DBUS_TYPE_VARIANT) {
				return std::nullopt;
			}

			DBusMessageIter variant_iter;
			dbus_message_iter_recurse(&reply_iter, &variant_iter);

			if (dbus_message_iter_get_arg_type(&variant_iter) != DBUS_TYPE_UINT32) {
				return std::nullopt;
			}

			dbus_uint32_t version = 0;
			dbus_message_iter_get_basic(&variant_iter, &version);
			return version;
		}

		static std::uint32_t query_interface_version(const char* const interface_name) {
			const auto bus = session_bus();

			if (bus.get() == nullptr) {
				return 0;
			}

			const auto version = get_interface_version(bus.get(), interface_name).value_or(0);

			LOG("Desktop portal: %x version %x.", interface_name, version);
			return version;
		}

		bool has_file_chooser() {
			return query_interface_version(file_chooser_interface) > 0;
		}

		bool can_open_directory() {
			return query_interface_version(open_uri_interface) >= first_version_opening_directories;
		}

		static std::string make_handle_token() {
			static std::atomic<unsigned> counter = 0;
			return "hypersomnia_" + std::to_string(::getpid()) + "_" + std::to_string(counter++);
		}

		static std::string expected_request_path(DBusConnection* const connection, const std::string& handle_token) {
			std::string sender = dbus_bus_get_unique_name(connection);

			if (!sender.empty() && sender.front() == ':') {
				sender.erase(sender.begin());
			}

			for (auto& c : sender) {
				if (c == '.') {
					c = '_';
				}
			}

			return std::string(object_path) + "/request/" + sender + "/" + handle_token;
		}

		static void listen_for_response(DBusConnection* const connection, const std::string& request_path) {
			const auto rule =
				"type='signal',interface='" + std::string(request_interface) +
				"',member='Response',path='" + request_path + "'"
			;

			dbus_bus_add_match(connection, rule.c_str(), nullptr);
		}

		static void listen_for_portal_leaving(DBusConnection* const connection) {
			const auto rule =
				"type='signal',sender='org.freedesktop.DBus',interface='org.freedesktop.DBus',"
				"member='NameOwnerChanged',arg0='" + std::string(bus_name) + "'"
			;

			dbus_bus_add_match(connection, rule.c_str(), nullptr);
		}

		static bool is_portal_leaving(DBusMessage* const message) {
			if (!dbus_message_is_signal(message, "org.freedesktop.DBus", "NameOwnerChanged")) {
				return false;
			}

			const char* name = nullptr;
			const char* old_owner = nullptr;
			const char* new_owner = nullptr;

			const bool read = dbus_message_get_args(
				message,
				nullptr,
				DBUS_TYPE_STRING, &name,
				DBUS_TYPE_STRING, &old_owner,
				DBUS_TYPE_STRING, &new_owner,
				DBUS_TYPE_INVALID
			);

			return read && name != nullptr && std::string(name) == bus_name && new_owner != nullptr && std::string(new_owner).empty();
		}

		static void append_string(DBusMessageIter& iter, const std::string& value) {
			const char* const c_str = value.c_str();
			dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &c_str);
		}

		template <class AppendValue>
		static void append_option(DBusMessageIter& dict_iter, const char* const key, const char* const signature, AppendValue append_value) {
			DBusMessageIter entry_iter;
			DBusMessageIter variant_iter;

			dbus_message_iter_open_container(&dict_iter, DBUS_TYPE_DICT_ENTRY, nullptr, &entry_iter);
			dbus_message_iter_append_basic(&entry_iter, DBUS_TYPE_STRING, &key);
			dbus_message_iter_open_container(&entry_iter, DBUS_TYPE_VARIANT, signature, &variant_iter);
			append_value(variant_iter);
			dbus_message_iter_close_container(&entry_iter, &variant_iter);
			dbus_message_iter_close_container(&dict_iter, &entry_iter);
		}

		static void append_string_option(DBusMessageIter& dict_iter, const char* const key, const std::string& value) {
			append_option(dict_iter, key, "s", [&value](DBusMessageIter& variant_iter) {
				append_string(variant_iter, value);
			});
		}

		static void append_null_terminated_bytes_option(DBusMessageIter& dict_iter, const char* const key, const std::string& value) {
			append_option(dict_iter, key, "ay", [&value](DBusMessageIter& variant_iter) {
				DBusMessageIter bytes_iter;
				dbus_message_iter_open_container(&variant_iter, DBUS_TYPE_ARRAY, "y", &bytes_iter);

				const auto* const bytes = value.c_str();
				dbus_message_iter_append_fixed_array(&bytes_iter, DBUS_TYPE_BYTE, &bytes, static_cast<int>(value.size() + 1));

				dbus_message_iter_close_container(&variant_iter, &bytes_iter);
			});
		}

		static void append_filters_option(DBusMessageIter& dict_iter, const std::vector<file_filter>& filters) {
			append_option(dict_iter, "filters", "a(sa(us))", [&filters](DBusMessageIter& variant_iter) {
				DBusMessageIter filters_iter;
				dbus_message_iter_open_container(&variant_iter, DBUS_TYPE_ARRAY, "(sa(us))", &filters_iter);

				for (const auto& filter : filters) {
					DBusMessageIter filter_iter;
					DBusMessageIter patterns_iter;

					dbus_message_iter_open_container(&filters_iter, DBUS_TYPE_STRUCT, nullptr, &filter_iter);
					append_string(filter_iter, filter.description);
					dbus_message_iter_open_container(&filter_iter, DBUS_TYPE_ARRAY, "(us)", &patterns_iter);

					for (const auto& pattern : filter.patterns) {
						DBusMessageIter pattern_iter;
						const dbus_uint32_t glob_pattern_type = 0;

						dbus_message_iter_open_container(&patterns_iter, DBUS_TYPE_STRUCT, nullptr, &pattern_iter);
						dbus_message_iter_append_basic(&pattern_iter, DBUS_TYPE_UINT32, &glob_pattern_type);
						append_string(pattern_iter, pattern);
						dbus_message_iter_close_container(&patterns_iter, &pattern_iter);
					}

					dbus_message_iter_close_container(&filter_iter, &patterns_iter);
					dbus_message_iter_close_container(&filters_iter, &filter_iter);
				}

				dbus_message_iter_close_container(&variant_iter, &filters_iter);
			});
		}

		static void append_file_chooser_options(
			DBusMessageIter& args_iter,
			const file_chooser_request& request,
			const std::string& handle_token
		) {
			DBusMessageIter dict_iter;
			dbus_message_iter_open_container(&args_iter, DBUS_TYPE_ARRAY, "{sv}", &dict_iter);

			append_string_option(dict_iter, "handle_token", handle_token);

			if (!request.filters.empty()) {
				append_filters_option(dict_iter, request.filters);
			}

			if (request.mode == file_chooser_mode::SAVE_FILE && !request.suggested_name.empty()) {
				append_string_option(dict_iter, "current_name", request.suggested_name);
			}

			if (!request.suggested_folder.empty()) {
				append_null_terminated_bytes_option(dict_iter, "current_folder", request.suggested_folder.string());
			}

			dbus_message_iter_close_container(&args_iter, &dict_iter);
		}

		static int hex_digit_value(const char c) {
			if (c >= '0' && c <= '9') {
				return c - '0';
			}

			if (c >= 'a' && c <= 'f') {
				return c - 'a' + 10;
			}

			if (c >= 'A' && c <= 'F') {
				return c - 'A' + 10;
			}

			return -1;
		}

		static std::optional<path_type> file_uri_to_path(const std::string& uri) {
			const auto scheme = std::string("file://");

			if (uri.rfind(scheme, 0) != 0) {
				LOG("Desktop portal: not a local file: %x", uri);
				return std::nullopt;
			}

			std::string decoded;

			for (std::size_t i = scheme.size(); i < uri.size(); ++i) {
				const bool percent_escape = uri[i] == '%' && i + 2 < uri.size();

				if (percent_escape) {
					const auto high = hex_digit_value(uri[i + 1]);
					const auto low = hex_digit_value(uri[i + 2]);

					if (high >= 0 && low >= 0) {
						decoded += static_cast<char>(high * 16 + low);
						i += 2;
						continue;
					}
				}

				decoded += uri[i];
			}

			return path_type(decoded);
		}

		static std::optional<std::string> first_uri_in_results(DBusMessageIter& results_iter) {
			DBusMessageIter dict_iter;
			dbus_message_iter_recurse(&results_iter, &dict_iter);

			while (dbus_message_iter_get_arg_type(&dict_iter) == DBUS_TYPE_DICT_ENTRY) {
				DBusMessageIter entry_iter;
				dbus_message_iter_recurse(&dict_iter, &entry_iter);

				const char* key = nullptr;
				dbus_message_iter_get_basic(&entry_iter, &key);
				dbus_message_iter_next(&entry_iter);

				if (key != nullptr && std::string(key) == "uris" && dbus_message_iter_get_arg_type(&entry_iter) == DBUS_TYPE_VARIANT) {
					DBusMessageIter variant_iter;
					dbus_message_iter_recurse(&entry_iter, &variant_iter);

					if (dbus_message_iter_get_arg_type(&variant_iter) == DBUS_TYPE_ARRAY) {
						DBusMessageIter uris_iter;
						dbus_message_iter_recurse(&variant_iter, &uris_iter);

						if (dbus_message_iter_get_arg_type(&uris_iter) == DBUS_TYPE_STRING) {
							const char* uri = nullptr;
							dbus_message_iter_get_basic(&uris_iter, &uri);

							if (uri != nullptr) {
								return std::string(uri);
							}
						}
					}
				}

				dbus_message_iter_next(&dict_iter);
			}

			return std::nullopt;
		}

		static std::optional<path_type> read_response(DBusMessage* const response) {
			DBusMessageIter iter;

			if (!dbus_message_iter_init(response, &iter) || dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_UINT32) {
				LOG("Desktop portal: malformed file chooser response.");
				return std::nullopt;
			}

			dbus_uint32_t response_code = 0;
			dbus_message_iter_get_basic(&iter, &response_code);

			if (response_code != response_success) {
				LOG("Desktop portal: file chooser closed without a choice (response %x).", response_code);
				return std::nullopt;
			}

			if (!dbus_message_iter_next(&iter) || dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY) {
				LOG("Desktop portal: file chooser response without results.");
				return std::nullopt;
			}

			if (const auto uri = first_uri_in_results(iter)) {
				return file_uri_to_path(*uri);
			}

			LOG("Desktop portal: file chooser response without a file.");
			return std::nullopt;
		}

		static std::optional<std::string> read_object_path(DBusMessage* const reply) {
			const char* path = nullptr;

			if (!dbus_message_get_args(reply, nullptr, DBUS_TYPE_OBJECT_PATH, &path, DBUS_TYPE_INVALID) || path == nullptr) {
				return std::nullopt;
			}

			return std::string(path);
		}

		static std::optional<path_type> wait_for_response(DBusConnection* const connection, const std::string& request_path) {
			while (dbus_connection_read_write(connection, response_poll_interval_ms)) {
				while (DBusMessage* const raw_message = dbus_connection_pop_message(connection)) {
					const auto message = message_ptr(raw_message);

					const bool is_our_response =
						dbus_message_is_signal(message.get(), request_interface, "Response") &&
						dbus_message_has_path(message.get(), request_path.c_str())
					;

					if (is_our_response) {
						return read_response(message.get());
					}

					if (is_portal_leaving(message.get())) {
						LOG("Desktop portal: the portal quit while waiting for the file chooser.");
						return std::nullopt;
					}
				}
			}

			LOG("Desktop portal: the session bus disconnected while waiting for the file chooser.");
			return std::nullopt;
		}

		std::optional<path_type> choose(const file_chooser_request& request) {
			const auto bus = session_bus();

			if (bus.get() == nullptr) {
				return std::nullopt;
			}

			const auto handle_token = make_handle_token();
			const auto request_path = expected_request_path(bus.get(), handle_token);

			listen_for_response(bus.get(), request_path);
			listen_for_portal_leaving(bus.get());

			const auto method = request.mode == file_chooser_mode::SAVE_FILE ? "SaveFile" : "OpenFile";
			const auto call = message_ptr(dbus_message_new_method_call(bus_name, object_path, file_chooser_interface, method));

			DBusMessageIter args_iter;
			dbus_message_iter_init_append(call.get(), &args_iter);

			append_string(args_iter, request.parent_window);
			append_string(args_iter, request.title);
			append_file_chooser_options(args_iter, request, handle_token);

			LOG("Desktop portal: opening the file chooser (%x).", method);

			const auto reply = call_and_wait_for_reply(bus.get(), call.get(), method);

			if (reply.get() == nullptr) {
				return std::nullopt;
			}

			const auto returned_path = read_object_path(reply.get());

			if (!returned_path.has_value()) {
				LOG("Desktop portal: %x returned no request handle.", method);
				return std::nullopt;
			}

			if (*returned_path != request_path) {
				listen_for_response(bus.get(), *returned_path);
			}

			return wait_for_response(bus.get(), *returned_path);
		}

		static int open_for_portal(const path_type& path) {
			const auto readable = ::open(path.c_str(), O_RDONLY | O_CLOEXEC);

			if (readable >= 0) {
				return readable;
			}

			return ::open(path.c_str(), O_PATH | O_CLOEXEC);
		}

		void open_directory_of(const path_type& path) {
			const auto bus = session_bus();

			if (bus.get() == nullptr) {
				return;
			}

			const auto fd = open_for_portal(path);

			if (fd < 0) {
				LOG("Desktop portal: could not open %x to show it in the file manager.", path);
				return;
			}

			const auto call = message_ptr(dbus_message_new_method_call(bus_name, object_path, open_uri_interface, "OpenDirectory"));

			DBusMessageIter args_iter;
			dbus_message_iter_init_append(call.get(), &args_iter);

			append_string(args_iter, "");
			dbus_message_iter_append_basic(&args_iter, DBUS_TYPE_UNIX_FD, &fd);

			DBusMessageIter dict_iter;
			dbus_message_iter_open_container(&args_iter, DBUS_TYPE_ARRAY, "{sv}", &dict_iter);
			dbus_message_iter_close_container(&args_iter, &dict_iter);

			::close(fd);

			LOG("Desktop portal: showing %x in the file manager.", path);
			call_and_wait_for_reply(bus.get(), call.get(), "OpenDirectory");
		}
	}
}
