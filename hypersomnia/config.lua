return {
  app_controls = {
    Tilde = "SWITCH_DEVELOPER_CONSOLE"
  },
  app_ingame_controls = {
    Backspace = "CLEAR_DEBUG_LINES",
    F = "SWITCH_WEAPON_LASER",
    MouseButton4 = "SWITCH_GAME_GUI_MODE",
	Tab = "OPEN_SCOREBOARD",
	F2 = "OPEN_BUY_MENU",
	F3 = "CHOOSE_TEAM"
  },
  audio = {
    enable_hrtf = false,
    max_number_of_sound_sources = 4096,
    output_device_name = "",
	sound_meters_per_second = 150
  },
  audio_volume = {
    gui = 1,
    music = 1,
    sound_effects = 1
  },
  sound = {
	sync_sounds_longer_than_secs = 5,
	max_divergence_before_sync_secs = 1
  },
  camera = {
    additional_position_smoothing = {
      average_factor = 0.5,
      averages_per_sec = 0
    },
    angled_look_length = 100,
    enable_smoothing = true,
    look_bound_expand = 0.5,
    smoothing = {
      average_factor = 0.5,
      averages_per_sec = 25
    }
  },
  content_regeneration = {
    regenerate_every_time = false
  },
  debug = {
    determinism_test_cloned_cosmoi_count = 0,
    input_recording_mode = "DISABLED"
  },
  debug_drawing = {
    draw_cast_rays = false,
    draw_colinearization = false,
    draw_discontinuities = false,
    draw_explosion_forces = false,
    draw_forces = false,
    draw_friction_field_collisions_of_entering = false,
    draw_memorised_walls = false,
    draw_triangle_edges = false,
    draw_undiscovered_locations = false,
    draw_visible_walls = false,
    enabled = false
  },
  drawing = {
    draw_character_gui = true,
    draw_crosshairs = true,
    draw_weapon_laser = true,
	disabled_draw_area_markers = 0.85,
	fog_of_war = {
	  overlay_color_on_visible = true,
	  overlay_color = "255 255 255 2",
	  enabled = true,
	  angle = 150
	},
	draw_enemy_hud = false
  },
  editor = {
	player = {
		snapshot_interval_in_steps = 800
	},
    grid = {
      render = {
        alpha_multiplier = 0.5,
        hide_grids_smaller_than = 16,
        line_colors = {
          "255 0 0 255",
          "255 0 255 255",
          "255 165 0 255",
          "255 255 0 100",
          "0 255 0 100",
          "0 255 255 100",
          "255 255 255 100",
          "177 177 177 100",
          "75 74 74 100",
          "54 54 54 100"
        },
        maximum_power_of_two = 12
      }
    },
    autosave = {
      enabled = true,
      once_every_min = 1
    },
    go_to = {
      dialog_width = 400,
      num_lines = 15,
    },
    test_scene = {
      scene_tickrate = 128,
      start_bomb_mode = true
    },
    camera = {
      panning_speed = 1
    },
    entity_selector = {
      hovered_color = "255 255 255 20",
      selected_color = "65 131 196 80",
      held_color = "65 131 196 120",
	  hovered_color = "255 255 255 80",
	  hovered_dashed_line_color = "255 255 255 140"
    },
	tutorial_text_color = "220 220 220 255",
    controlled_entity_color = "255 255 0 120",
    matched_entity_color = "0 255 0 80",

    rectangular_selection_color = "65 131 196 60",
    rectangular_selection_border_color = "65 131 196 120",

	action_indicator = {
		bg_color = "0 0 0 180",
		bg_border_color = "255 255 255 15",

		max_width = 300,
		show_for_ms = 3000,
		text_padding = { x = 10, y = 10 },
		offset = { x = 80, y = 80 },
	},

    different_values_frame_bg = "115 73 0 255",
    different_values_frame_hovered_bg = "158 122 0 255",
    different_values_frame_active_bg = "168 158 0 255"
  },
  input = {
	mouse_sensitivity = {
	  x = 3,
	  y = 3
	}
  },
  game_controls = {
    A = "MOVE_LEFT",
    D = "MOVE_RIGHT",
    E = "USE_BUTTON",
    G = "DROP",
    MiddleMouseButton = "THROW",
    LeftControl = "START_PICKING_UP_ITEMS",
    LeftMouseButton = "CROSSHAIR_PRIMARY_ACTION",
    LeftShift = "SPRINT",
    RightMouseButton = "CROSSHAIR_SECONDARY_ACTION",
    S = "MOVE_BACKWARD",
    Space = "SPACE_BUTTON",
    W = "MOVE_FORWARD",
	R = "RELOAD"
  },
  game_gui_controls = {
    ["0"] = "HOTBAR_BUTTON_9",
    ["1"] = "HOTBAR_BUTTON_0",
    ["2"] = "HOTBAR_BUTTON_1",
    ["3"] = "HOTBAR_BUTTON_2",
    ["4"] = "HOTBAR_BUTTON_3",
    ["5"] = "HOTBAR_BUTTON_4",
    ["6"] = "HOTBAR_BUTTON_5",
    ["7"] = "HOTBAR_BUTTON_6",
    ["8"] = "HOTBAR_BUTTON_7",
    ["9"] = "HOTBAR_BUTTON_8",
    B = "SPECIAL_ACTION_BUTTON_5",
    C = "SPECIAL_ACTION_BUTTON_3",
    F2 = "SPECIAL_ACTION_BUTTON_8",
    F3 = "SPECIAL_ACTION_BUTTON_9",
    F4 = "SPECIAL_ACTION_BUTTON_10",
    F5 = "SPECIAL_ACTION_BUTTON_11",
    F6 = "SPECIAL_ACTION_BUTTON_12",
    H = "HOLSTER",
    M = "SPECIAL_ACTION_BUTTON_7",
    N = "SPECIAL_ACTION_BUTTON_6",
    Q = "PREVIOUS_HOTBAR_SELECTION_SETUP",
    V = "SPECIAL_ACTION_BUTTON_4",
    X = "SPECIAL_ACTION_BUTTON_2",
    Z = "SPECIAL_ACTION_BUTTON_1"
  },
  gui_fonts = {
	  gui = {
		unicode_ranges = {
		  { 0x0020, 0x00FF }, -- Basic Latin + Latin Supplement
		  { 0x0100, 0x017F }  -- Latin Extended-A
		},
		size_in_pixels = 16,
		add_japanese_ranges = false,
		settings = {},
		source_font_path = "content/necessary/fonts/unifont.ttf"
	  },
	  large_numbers = {
		unicode_ranges = {
		  { 0x0030, 0x0039 }, -- Only numbers
		},
		size_in_pixels = 64,
		add_japanese_ranges = false,
		settings = {},
		source_font_path = "content/necessary/fonts/unifont.ttf"
	  }
  },
  gui_style = {
    Alpha = 1,
    AntiAliasedLines = true,
    AntiAliasedShapes = true,
    ButtonTextAlign = {
      x = 0.5,
      y = 0.5
    },
    ChildBorderSize = 1,
    ChildRounding = 0,
    ColumnsMinSpacing = 6,
    CurveTessellationTol = 1.25,
    DisplaySafeAreaPadding = {
      x = 4,
      y = 4
    },
    DisplayWindowPadding = {
      x = 22,
      y = 22
    },
    FrameBorderSize = 0,
    FramePadding = {
      x = 4,
      y = 3
    },
    FrameRounding = 0,
    GrabMinSize = 10,
    GrabRounding = 0,
    ImGuiCol_Border = "109 109 127 127",
    ImGuiCol_BorderShadow = "0 0 0 0",
    ImGuiCol_Button = "66 150 249 102",
    ImGuiCol_ButtonActive = "15 135 249 255",
    ImGuiCol_ButtonHovered = "66 150 249 255",
    ImGuiCol_CheckMark = "66 150 249 255",
    ImGuiCol_ChildBg = "255 255 255 0",
    ImGuiCol_DragDropTarget = "255 255 0 229",
    ImGuiCol_FrameBg = "40 73 122 137",
    ImGuiCol_FrameBgActive = "66 150 249 170",
    ImGuiCol_FrameBgHovered = "66 150 249 102",
    ImGuiCol_Header = "66 150 249 79",
    ImGuiCol_HeaderActive = "66 150 249 255",
    ImGuiCol_HeaderHovered = "66 150 249 204",
    ImGuiCol_MenuBarBg = "35 35 35 255",
    ImGuiCol_ModalWindowDarkening = "204 204 204 89",
    ImGuiCol_PlotHistogram = "229 178 0 255",
    ImGuiCol_PlotHistogramHovered = "255 153 0 255",
    ImGuiCol_PlotLines = "155 155 155 255",
    ImGuiCol_PlotLinesHovered = "255 109 89 255",
    ImGuiCol_PopupBg = "20 20 20 239",
    ImGuiCol_ResizeGrip = "66 150 249 63",
    ImGuiCol_ResizeGripActive = "66 150 249 242",
    ImGuiCol_ResizeGripHovered = "66 150 249 170",
    ImGuiCol_ScrollbarBg = "5 5 5 135",
    ImGuiCol_ScrollbarGrab = "79 79 79 255",
    ImGuiCol_ScrollbarGrabActive = "130 130 130 255",
    ImGuiCol_ScrollbarGrabHovered = "104 104 104 255",
    ImGuiCol_Separator = "109 109 127 127",
    ImGuiCol_SeparatorActive = "25 102 191 255",
    ImGuiCol_SeparatorHovered = "25 102 191 198",
    ImGuiCol_SliderGrab = "61 132 224 255",
    ImGuiCol_SliderGrabActive = "66 150 249 255",
    ImGuiCol_Text = "255 255 255 255",
    ImGuiCol_TextDisabled = "127 127 127 255",
    ImGuiCol_TextSelectedBg = "66 150 249 89",
    ImGuiCol_TitleBg = "10 10 10 255",
    ImGuiCol_TitleBgActive = "40 73 122 255",
    ImGuiCol_TitleBgCollapsed = "0 0 0 130",
    ImGuiCol_WindowBg = "15 15 15 239",
    IndentSpacing = 21,
    ItemInnerSpacing = {
      x = 4,
      y = 4
    },
    ItemSpacing = {
      x = 8,
      y = 4
    },
    PopupBorderSize = 1,
    PopupRounding = 0,
    ScrollbarRounding = 0,
    ScrollbarSize = 16,
    TouchExtraPadding = {
      x = 0,
      y = 0
    },
    WindowBorderSize = 1,
    WindowMinSize = {
      x = 32,
      y = 32
    },
    WindowPadding = {
      x = 8,
      y = 8
    },
    WindowRounding = 0,
    WindowTitleAlign = {
      x = 0,
      y = 0.5
    }
  },
  arena_mode_gui = {
    between_knockout_boxes_pad = 6,
    inside_knockout_box_pad = 4,
    weapon_icon_horizontal_pad = 10,
    show_recent_knockouts_num = 7,
    keep_recent_knockouts_for_seconds = 8,

	money_indicator_color = "255 249 133 255",
	award_indicator_color = "255 255 0 255",

	show_recent_awards_num = 8,
	keep_recent_awards_for_seconds = 3,

	money_indicator_pos = {
		x = -50,
		y = 20 + 16 * 8
	},

	buy_menu_settings = {
		disabled_bg = "75 0 0 180",
		disabled_active_bg = "90 0 0 220",
		already_owns_active_bg = "0 130 0 150",
		already_owns_bg = "0 76 0 91"
	},

    scoreboard_settings = {
	  dark_color_overlay_under_score = true,
      background_color = "15 15 15 200",
      bg_lumi_mult = 0.69999998807907104,
      border_color = "109 109 127 100",
      current_player_bg_lumi_mult = 1.6000000238418579,
      current_player_text_lumi_mult = 1.7000000476837158,
      dead_player_bg_alpha_mult = 0.30000001192092896,
      dead_player_bg_lumi_mult = 0.5,
      dead_player_text_alpha_mult = 0.60000002384185791,
      dead_player_text_lumi_mult = 0.80000001192092896,
      cell_bg_alpha = 0.56000000238418579,
      faction_logo_alpha = 0.80000001192092896,
      player_row_inner_padding = {
        x = 4,
        y = 4
      },
      text_lumi_mult = 1.7000000476837158,
      text_stroke_lumi_mult = 0.5,
      window_padding = {
        x = 4,
        y = 8
      }
    },
  },
  hotbar = {
    colorize_inside_when_selected = true,
    increase_inside_alpha_when_selected = false,
    primary_selected_color = "0 255 255 255",
    secondary_selected_color = "86 156 214 255"
  },
  game_gui = {

  },
  interpolation = {
    enabled = true,
    speed = 1000
  },
  launch_mode = "MAIN_MENU",
  main_menu = {
    latest_news_url = "http://hypersomnia.xyz/latest_post/",
    menu_intro_scene_entropy_path = "abc",
    menu_intro_scene_intercosm_path = "abc",
    menu_theme_path = "",
    rewind_intro_scene_by_secs = 3.5,
    skip_credits = true,
    start_menu_music_at_secs = 0
  },
  session = {
    automatically_hide_settings_ingame = false,
    show_developer_console = false,
    camera_query_aabb_mult = 1.0
  },
  test_scene = {
    create_minimal_test_scene = false,
    scene_tickrate = 128
  },
  simulation_receiver = {
    misprediction_smoothing_multiplier = 1.2000000476837158
  },
  unit_tests = {
    break_on_failure = true,
    log_successful = false,
    redirect_log_to_path = "",
    run = true
  },
  window = {
    app_icon_path = "content/necessary/gfx/app.ico",
    border = true,
    bpp = 24,
    fullscreen = false,
    name = "Hypersomnia",
    position = {
      x = 189,
      y = 208
    },
    raw_mouse_input = false,
    log_keystrokes = false,
    size = {
      x = 1200,
      y = 700
    },
	vsync_mode = OFF
  },
  faction_view = {
	colors = {
      SPECTATOR = {
		  standard = "200 200 200 255",
		  background_dark = "0 0 0 0"
	  },
      METROPOLIS = {
		  standard = "200 0 255 255",
		  background_dark = "20 0 75 255"
	  },
      ATLANTIS = {
		  standard = "0 255 0 255"
	  },
      RESISTANCE = {
		  standard = "255 37 0 255",
		  background_dark = "47 10 0 255"
	  }
	}
  },

  default_server_start = {
	ip = "127.0.0.1",
	port = 8412,
	max_connections = 64
  },

  server = {
	current_arena = "test",
	kick_if_inactive_for_secs = 60,
	time_limit_to_enter_game_since_connection = 10
  },

  default_client_start = {
	ip_port = "127.0.0.1:8412",
  },

  client = {
	nickname = "Player",
	requested_jitter_buffer_ms = 33
  }
}
