#include "application/gui/leaderboards_gui.h"

#include "application/gui/leaderboards_gui.h"
#include "augs/network/netcode_utils.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "3rdparty/include_httplib.h"
#include "augs/templates/thread_templates.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/pointer_to_buffer.h"
#include "augs/readwrite/byte_readwrite.h"
#include "application/setups/client/client_connect_string.h"
#include "augs/log.h"
#include "application/setups/debugger/detail/maybe_different_colors.h"
#include "augs/misc/date_time.h"
#include "augs/network/netcode_sockets.h"
#include "application/nat/nat_detection_settings.h"
#include "application/network/resolve_address.h"
#include "application/masterserver/masterserver_requests.h"
#include "application/masterserver/gameserver_command_readwrite.h"
#include "augs/misc/httplib_utils.h"
#include "application/network/resolve_address.h"
#include "augs/readwrite/json_readwrite.h"
#include "application/setups/editor/detail/simple_two_tabs.h"
#include "augs/window_framework/shell.h"

#include "augs/misc/imgui/imgui_game_image.h"
#include "view/ranks_info.h"

double yojimbo_time();

struct leaderboards_gui_internal {
	std::future<std::optional<httplib::Result>> future_response;

	bool refresh_op_in_progress() const {
		return future_response.valid();
	}
};

leaderboards_gui_state::~leaderboards_gui_state() = default;

leaderboards_gui_state::leaderboards_gui_state(const std::string& title) 
	: base(title), data(std::make_unique<leaderboards_gui_internal>()) 
{

}

double yojimbo_time();

static all_leaderboards to_players_list(std::optional<httplib::Result> result, std::string& error_message) {
    using namespace httplib_utils;

    if (result == std::nullopt || result.value() == nullptr) {
        error_message = "Couldn't connect to the server list host.";
        return {};
    }

    const auto& response = result.value();
    const auto status = response->status;

    LOG("Leaderboards response status: %x", status);

    if (!successful(status)) {
        const auto couldnt_download = std::string("Couldn't download the leaderboards.\n");

        error_message = couldnt_download + "HTTP response: " + std::to_string(status);
        return {};
    }

    const auto& json = response->body;

    LOG("Leaderboards response json length: %x", json.size());

	try {
		return augs::from_json_string<all_leaderboards>(json);
	}
	catch (const std::runtime_error& err) {
		error_message = err.what();
	}
	catch (...) {
		error_message = "Unknown error during deserialization.";
	}

	return all_leaderboards();
}

bool leaderboards_gui_state::refresh_in_progress() const {
	return data->refresh_op_in_progress();
}

void leaderboards_gui_state::refresh_leaderboards(const leaderboards_input in) {
	using namespace httplib;

	if (refresh_in_progress()) {
		return;
	}

	error_message.clear();

	LOG("Launching future_response");

	if (const auto parsed = parsed_url(in.provider_url); parsed.valid()) {
		data->future_response = launch_async(
			[parsed]() -> std::optional<httplib::Result> {
				using namespace httplib_utils;

				LOG_NVPS(parsed.host, parsed.protocol, parsed.port, parsed.location);
				auto http_client = make_client(parsed);

				const auto final_location = parsed.location + "?format=json";

				return launch_download(*http_client, final_location.c_str());
			}
		);
	}
	else {
		LOG("Invalid leaderboards provider: %x", in.provider_url);
	}

	LOG("refresh_leaderboards returns.");
}

void leaderboards_gui_state::perform(const leaderboards_input in) {
	using namespace httplib_utils;

	/* Always visible */
	show = true;

	if (refresh_requested) {
		refresh_requested = false;
		refresh_leaderboards(in);
	}

	if (valid_and_is_ready(data->future_response)) {
		all = ::to_players_list(data->future_response.get(), error_message);

		for (auto& s : all.leaderboards_team) {
			s.is_us = typesafe_sprintf("steam_%x", in.steam_id) == s.account_id;
		}

		for (auto& s : all.leaderboards_ffa) {
			s.is_us = typesafe_sprintf("steam_%x", in.steam_id) == s.account_id;
		}

		refreshed_once = true;
	}

	if (!show) {
		return;
	}

	using namespace augs::imgui;

	const auto screen_size = vec2(ImGui::GetIO().DisplaySize);
	const auto w = screen_size.x/2.5f;
	//const auto h = screen_size.y/1.3f;
	const auto padding = vec2i(70-14, 70-10);

	const auto h = std::min(int(screen_size.y-padding.y*2), 766);//;in.menu_ltrb.h();

	const auto rb_pos = ImVec2(screen_size.x - w - padding.x,screen_size.y - h - padding.y);
	const auto menu_pos = ImVec2(in.menu_ltrb.r, in.menu_ltrb.t);
	(void)rb_pos;
	(void)menu_pos;

	const auto col0_w = ImGui::CalcTextSize("Place 99").x;
	const auto col1_w = ImGui::CalcTextSize("9").x * max_nickname_length_v;
	const auto col2_w = ImGui::CalcTextSize("MMR (OpenSkill) 99999").x;

	const auto approx_contwidth = col0_w + col1_w + col2_w;

	ImGui::SetNextWindowSize(ImVec2(w,h), ImGuiCond_Always);
	ImGui::SetNextWindowPos(rb_pos, ImGuiCond_Always);

	const auto flags = 
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize
	;

	auto imgui_window = augs::imgui::scoped_window("Leaderboards", &show, flags);

	if (!imgui_window) {
		return;
	}

	if (!error_message.empty()) {
		text_color(typesafe_sprintf("Failed to download leaderboards from:\n%x\n%x", in.provider_url, error_message), red);

		if (ImGui::Button("Try to refresh")) {
			request_refresh();
		}

		return;
	}

	thread_local ImGuiTextFilter filter;

#if 0
	if (!all.leaderboards_team.empty()) {
		all.leaderboards_team[0].mmr = 62.498f;
		all.leaderboards_team[1].mmr = 51.847f;
	}
#endif

	float our_mmr = 0.0f;

	auto get_current_list = [&]() -> std::vector<leaderboards_entry>& {
		if (type == leaderboards_type::TEAM) {
			return all.leaderboards_team;
		}

		return all.leaderboards_ffa;
	};

	bool played = false;
	std::size_t our_place = 0;

	for (const auto& s : get_current_list()) {
		if (s.is_us) {
			our_mmr = s.mmr;
			our_place = 1 + index_in(get_current_list(), s);
			played = true;
		}
	}

#if 0
	our_mmr = yojimbo_time()*2.5f-20.0f;
#endif

	if (refresh_in_progress() && !refreshed_once) {
		text_color("Downloading leaderboards...", yellow);
		return;
	}

	if (refresh_in_progress()) {
		const auto secs = yojimbo_time();

		const auto num_dots = uint64_t(secs * 3) % 3 + 1;
		const auto loading_dots = std::string(num_dots, '.');

		text_color(std::string("Refreshing")+loading_dots, yellow);
	}
	else {
		text_color(typesafe_sprintf("Welcome home,", in.nickname), green);
	}

	ImGui::Columns(3);

	ImGui::SetColumnWidth(0, max_avatar_side_v + 10);

	const auto col_h = ImGui::GetTextLineHeight();

	{
		const auto one_letter = ImGui::CalcTextSize("9");
		const auto nick_w = ImGui::CalcTextSize(in.nickname.c_str()).x + one_letter.x*4;
		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
		const auto mmr_w = ImGui::CalcTextSize("MMR: 9.99999999").x;
		ImGui::PopFont();
		const auto col_w = std::max(nick_w, mmr_w);

		ImGui::SetColumnWidth(1, col_w);
	}

	{
		const auto avatar_size = vec2(max_avatar_side_v, max_avatar_side_v);

		augs::atlas_entry entry;
		entry.atlas_space = xywh(0, 0, 1, 1);

		if (augs::imgui::game_image_button("##AvatarButton", entry, avatar_size, {}, augs::imgui_atlas_type::AVATAR_PREVIEW)) {

		}
	}

	{
		auto scope = scoped_style_color(ImGuiCol_Separator, ImVec4(0,0,0,0));
		ImGui::NextColumn();
	}

	{
		//auto cur = scoped_preserve_cursor();
		text(in.nickname);
	}

	{
		//ImGui::SetCursorPos(ImGui::GetCursorPos);

	}

	const auto place_col = [&](auto place) {
		if (place == 1) {
			return  rgba(255,215,0, 255);
		}
#if 0
		if (place == 2) {
			return  rgba(192,192,192, 255);
		}
		if (place == 3) {
			return rgba(205,127,50, 255);
		}
#endif

		return white;
	};


	const auto mmr_text = "MMR:";
	const auto mmr_num_text = typesafe_sprintf("%4f", our_mmr);

	const auto our_rank = ::get_rank_for(our_mmr);

	if (played) {
		text("Place:");
		ImGui::SameLine();
		text_color(std::string("#")+std::to_string(our_place), place_col(our_place));


		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

		text(mmr_text);
		ImGui::SameLine();
		text_color(mmr_num_text, our_rank.number_color);
		ImGui::PopFont();
	}
	else {
		text("No matches yet!");

		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

		text(mmr_text);
		ImGui::SameLine();
		text_disabled(mmr_num_text);
		ImGui::PopFont();
	}

	ImGui::NextColumn();

	{
		auto crs = scoped_preserve_cursor();

		const auto entry = in.necessary_images.at(assets::necessary_image_id::RANK_BACKGROUND);

		auto cols = colors_nha {
			our_rank.name_color,
			rgba(our_rank.name_color).mult_luminance(1.1f),
			rgba(our_rank.name_color).mult_luminance(1.2f)
		};

		augs::imgui::game_image_button("##RankBg", entry, entry.get_original_size(), cols, augs::imgui_atlas_type::GAME);
	}

	{
		const auto entry = in.necessary_images.at(our_rank.icon);

		augs::imgui::game_image_button("##RankIcon", entry, entry.get_original_size(), {}, augs::imgui_atlas_type::GAME);
	}

	ImGui::SameLine();

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	text_color(our_rank.name, our_rank.name_color);
	ImGui::PopFont();

	ImGui::NextColumn();

	ImGui::Columns(1);

	ImGui::Separator();

	auto show_leaderboards = [&](const auto& list) {
		{
			auto sc = scoped_style_color(ImGuiCol_FrameBg, rgba(255,255,255,10));

			filter_with_hint(filter, "##HierarchyFilter", "Search players...");
		}

		ImGui::SetNextWindowContentSize(ImVec2(approx_contwidth, 0));

		auto child = scoped_child("players view", ImVec2(0,0), false, ImGuiWindowFlags_HorizontalScrollbar);

		ImGui::Columns(3);
		ImGui::SetColumnWidth(0, col0_w);
		ImGui::SetColumnWidth(1, col1_w);
		ImGui::SetColumnWidth(2, col2_w);

		text_disabled("Place");
		ImGui::NextColumn();
		text_disabled("Name");
		ImGui::NextColumn();
		text_disabled("MMR (OpenSkill)");
		ImGui::NextColumn();

		ImGui::Separator();

		const auto& item_spacing = ImGui::GetStyle().ItemSpacing;
		auto sv = scoped_style_var(ImGuiStyleVar_ItemSpacing, ImVec2(item_spacing.x,item_spacing.y*2.5f));

		for (const auto& s : list) {
			auto open_url = [&]() {
				augs::open_url(typesafe_sprintf("https://hypersomnia.xyz/user/%x", s.account_id));
			};

			const auto place = 1 + index_in(list, s);
			const auto col = place_col(place);

			const auto& name = s.nickname;
			const auto effective_name = std::string(name) + " " + std::string(s.account_id);

			if (!filter.PassFilter(effective_name.c_str())) {
				continue;
			}

			const bool is_us = typesafe_sprintf("steam_%x", in.steam_id) == s.account_id;
			const auto place_str = std::to_string(place);

			auto sc = scoped_text_color(col);

			if (place <= 1) {
				text(place_str);
			}
			else {
				text_disabled(place_str);
			}

			ImGui::NextColumn();

			{

				bool viewing = false;

				const auto this_rank = ::get_rank_for(s.mmr);

				auto do_hovered_logic = [&]() {
					if (viewing) {
						return;
					}

					viewing = true;

					auto sc = scoped_tooltip();

					{
						auto crs = scoped_preserve_cursor();

						const auto entry = in.necessary_images.at(assets::necessary_image_id::RANK_BACKGROUND);

						const auto cols = colors_nha {
							this_rank.name_color,
							this_rank.name_color,
							this_rank.name_color
						};

						augs::imgui::game_image_button("##RankBgSub", entry, entry.get_original_size(), cols, augs::imgui_atlas_type::GAME);
					}

					const auto entry = in.necessary_images.at(this_rank.icon);

					augs::imgui::game_image_button("##RankIcon", entry, entry.get_original_size(), {}, augs::imgui_atlas_type::GAME);
					ImGui::SameLine();

					ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
					text_color(this_rank.name, this_rank.name_color);
					ImGui::PopFont();
				};

				{
					const auto entry = in.necessary_images.at(this_rank.icon);
					const auto sz = vec2::scaled_to_max_size(entry.get_original_size(), col_h*1.7f); 

					if (augs::imgui::game_image_button("##RankMiniIcon", entry, sz, {}, augs::imgui_atlas_type::GAME)) {
						open_url();
					}

					if (ImGui::IsItemHovered()) {
						do_hovered_logic();
					}

					ImGui::SameLine();
				}

				const auto alpha_sin = (std::sin(yojimbo_time()*2) + 1) / 2;
				auto col_sin = this_rank.name_color;
				col_sin.a =  alpha_sin*30+20;

				if (!is_us) {
					col_sin.a = 60;
				}

				auto this_rank_hov = this_rank.name_color;
				this_rank_hov.a = 90;

				auto this_rank_act = this_rank.name_color;
				this_rank_act.a = 120;

				auto every_two = place % 2 == 0;

				if (every_two && !is_us && !viewing) {
					col_sin = rgba(255, 255, 255, 8);
				}

				auto progress_bg_cols = scoped_selectable_colors({ col_sin, this_rank_hov, this_rank_act });


				if (ImGui::Selectable(s.nickname.c_str(), every_two || is_us || viewing, ImGuiSelectableFlags_SpanAllColumns)) {
					open_url();
				}

				if (ImGui::IsItemHovered()) {
					do_hovered_logic();
				}
			}


			ImGui::NextColumn();

			text(typesafe_sprintf("%2f", s.mmr));

			ImGui::NextColumn();
		}
	};

	simple_two_tabs(
		type,
		leaderboards_type::TEAM,
		leaderboards_type::FFA,
		"Bomb Defusal",
		"FFA",
		[]() {},

		{
			rgba(0, 0, 0, 0),
			rgba(255, 255, 255, 30),
			rgba(255, 255, 255, 60)
		},
		{
			rgba(255, 255, 255, 20),
			rgba(255, 255, 255, 30),
			rgba(255, 255, 255, 60)
		}
	);

	ImGui::Separator();

	show_leaderboards(get_current_list());
}
