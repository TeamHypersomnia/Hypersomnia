return {
  audio = {
    enable_hrtf = false,
    max_number_of_sound_sources = 4096,
    output_device_name = ""
  },
  audio_volume = {
    gui = 1,
    music = 1,
    sound_effects = 1
  },
  camera = {
    angled_look_length = 100,
    averages_per_sec = 25,
    enable_smoothing = true,
    smooth_value_field_settings = {
      averages_per_sec = 0,
      smoothing_average_factor = 0.5
    },
    smoothing_average_factor = 0.5
  },
  content_regeneration = {
    check_integrity_every_launch = true,
    hotbar_button = {
      border_color = "255 255 255 255",
      inside_color = "255 255 255 255",
      lower_side = 20,
      upper_side = 8,
      inside_border_padding = 4,
      make_lb_complement = true
    },
    menu_button = {
      border_color = "255 255 255 255",
      inside_color = "255 255 255 255",
      lower_side = 12,
      upper_side = 8,
      inside_border_padding = 4,
      make_lb_complement = false
    },
    packer_detail_max_atlas_size = 8192,
    regenerate_every_launch = false,
    save_regenerated_atlases_as_binary = true
  },
  controls = {
    key_to_intent = {
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
      A = "MOVE_LEFT",
      B = "SPECIAL_ACTION_BUTTON_5",
      Backspace = "CLEAR_DEBUG_LINES",
      C = "SPECIAL_ACTION_BUTTON_3",
      CapsLock = "DEBUG_SWITCH_CHARACTER",
      D = "MOVE_RIGHT",
      Dash = "OPEN_DEVELOPER_CONSOLE",
      E = "USE_BUTTON",
      F = "SWITCH_WEAPON_LASER",
      F2 = "SPECIAL_ACTION_BUTTON_8",
      F3 = "SPECIAL_ACTION_BUTTON_9",
      F4 = "SPECIAL_ACTION_BUTTON_10",
      F5 = "SPECIAL_ACTION_BUTTON_11",
      F6 = "SPECIAL_ACTION_BUTTON_12",
      G = "THROW",
      H = "HOLSTER",
      LeftControl = "START_PICKING_UP_ITEMS",
      LeftMouseButton = "CROSSHAIR_PRIMARY_ACTION",
      LeftShift = "SPRINT",
      M = "SPECIAL_ACTION_BUTTON_7",
      MouseButton4 = "SWITCH_TO_GUI",
      N = "SPECIAL_ACTION_BUTTON_6",
      Q = "PREVIOUS_HOTBAR_SELECTION_SETUP",
      RightMouseButton = "CROSSHAIR_SECONDARY_ACTION",
      S = "MOVE_BACKWARD",
      Space = "SPACE_BUTTON",
      V = "SPECIAL_ACTION_BUTTON_4",
      W = "MOVE_FORWARD",
      X = "SPECIAL_ACTION_BUTTON_2",
      Z = "SPECIAL_ACTION_BUTTON_1"
    },
    map_mouse_motion_to = "MOVE_CROSSHAIR"
  },
  debug = {
    create_minimal_test_scene = false,
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
    show_profile_details = true
  },
  gui_style = {
    Alpha = 1,
    AntiAliasedLines = false,
    AntiAliasedShapes = false,
    ButtonTextAlign = {
      x = 0.5,
      y = 0.5
    },
    ChildWindowRounding = 0,
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
    FramePadding = {
      x = 4,
      y = 3
    },
    FrameRounding = 0,
    GrabMinSize = 10,
    GrabRounding = 0,
    ImGuiCol_Border = "178 178 178 165",
    ImGuiCol_BorderShadow = "0 0 0 0",
    ImGuiCol_Button = "187 35 219 179",
    ImGuiCol_ButtonActive = "255 105 137 255",
    ImGuiCol_ButtonHovered = "242 65 165 255",
    ImGuiCol_CheckMark = "229 229 229 127",
    ImGuiCol_ChildWindowBg = "0 0 0 0",
    ImGuiCol_CloseButton = "127 127 229 127",
    ImGuiCol_CloseButtonActive = "178 178 178 255",
    ImGuiCol_CloseButtonHovered = "178 178 229 153",
    ImGuiCol_Column = "127 127 127 255",
    ImGuiCol_ColumnActive = "229 178 178 255",
    ImGuiCol_ColumnHovered = "178 153 153 255",
    ImGuiCol_ComboBg = "51 51 51 252",
    ImGuiCol_FrameBg = "204 204 204 76",
    ImGuiCol_FrameBgActive = "229 165 165 114",
    ImGuiCol_FrameBgHovered = "229 204 204 102",
    ImGuiCol_Header = "102 102 229 114",
    ImGuiCol_HeaderActive = "135 135 221 204",
    ImGuiCol_HeaderHovered = "114 114 229 204",
    ImGuiCol_MenuBarBg = "102 102 140 204",
    ImGuiCol_ModalWindowDarkening = "51 51 51 89",
    ImGuiCol_PlotHistogram = "229 178 0 255",
    ImGuiCol_PlotHistogramHovered = "255 153 0 255",
    ImGuiCol_PlotLines = "255 255 255 255",
    ImGuiCol_PlotLinesHovered = "229 178 0 255",
    ImGuiCol_PopupBg = "12 12 25 229",
    ImGuiCol_ResizeGrip = "255 255 255 76",
    ImGuiCol_ResizeGripActive = "255 255 255 229",
    ImGuiCol_ResizeGripHovered = "255 255 255 153",
    ImGuiCol_ScrollbarBg = "51 63 76 153",
    ImGuiCol_ScrollbarGrab = "102 102 204 76",
    ImGuiCol_ScrollbarGrabActive = "204 127 127 102",
    ImGuiCol_ScrollbarGrabHovered = "102 102 204 102",
    ImGuiCol_SliderGrab = "255 255 255 76",
    ImGuiCol_SliderGrabActive = "204 127 127 255",
    ImGuiCol_Text = "229 229 229 255",
    ImGuiCol_TextDisabled = "153 153 153 255",
    ImGuiCol_TextSelectedBg = "0 0 255 89",
    ImGuiCol_TitleBg = "27 59 165 179",
    ImGuiCol_TitleBgActive = "0 81 165 221",
    ImGuiCol_TitleBgCollapsed = "35 44 165 151",
    ImGuiCol_WindowBg = "0 0 0 228",
    IndentSpacing = 21,
    ItemInnerSpacing = {
      x = 4,
      y = 4
    },
    ItemSpacing = {
      x = 8,
      y = 4
    },
    ScrollbarRounding = 0,
    ScrollbarSize = 16,
    TouchExtraPadding = {
      x = 0,
      y = 0
    },
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
  hotbar = {
    colorize_inside_when_selected = true,
    increase_inside_alpha_when_selected = false,
    primary_selected_color = "0 255 255 255",
    secondary_selected_color = "86 156 214 255"
  },
  interpolation = {
    enabled = true,
    speed = 525
  },
  launch_mode = "LOCAL",
  main_menu = {
    latest_news_url = "http://hypersomnia.pl/latest_post/",
    menu_intro_scene_cosmos_path = "",
    menu_intro_scene_entropy_path = "",
    menu_theme_path = "",
    rewind_intro_scene_by_secs = 3.5,
    skip_credits = true,
    start_menu_music_at_secs = 0
  },
  simulation_receiver = {
    misprediction_smoothing_multiplier = 1.2
  },
  unit_tests = {
    break_on_failure = true,
    log_successful = false,
    output_log_path = "generated/logs/unit_tests.txt",
    run = true
  },
  window = {
    border = true,
    bpp = 24,
    enable_cursor_clipping = true,
    fullscreen = false,
    name = "Hypersomnia",
    position = {
      x = 100,
      y = 10
    },
    size = {
      x = 1280,
      y = 768
    }
  }
}