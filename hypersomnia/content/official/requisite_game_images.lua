{
	hotbar_button = {
		source_image_path = "official/gfx/generated/procedural/hotbar_button.png",

		button_with_corners = {
			border_color = "255 255 255 255",
			inside_color = "255 255 255 255",
			lower_side = 20,
			upper_side = 8,
			inside_border_padding = 4,
			make_lb_complement = true
		}
	},

	menu_button = {
		source_image_path = "official/gfx/generated/procedural/menu_button.png",

		button_with_corners = {
			border_color = "255 255 255 255",
			inside_color = "255 255 255 255",
			lower_side = 12,
			upper_side = 8,
			inside_border_padding = 4,
			make_lb_complement = false
		}
	},

	attachment_circle_filled = {
		source_image_path = "official/gfx/generated/procedural/attachment_circle_filled.png",

		scripted_image = {
		commands = {
			{
				command = "circle_filled",
				input = {
					radius = 16,
					filling = "255 255 255 255"
				}
			}
			}
		}
	},

	attachment_circle_border = {
		source_image_path = "official/gfx/generated/procedural/attachment_circle_border.png",

		scripted_image = {
		commands = {
			{
				command = "circle_midpoint",
				input = {
					radius = 16,
					border_width = 1,		
					scale_alpha = false,
					constrain_angle = false,
					angle_start = 0,
					angle_end = 0,
					filling = "255 255 255 255"
				}
			}
			}
		}
	},

	action_button_filled = {
		source_image_path = "official/gfx/generated/procedural/action_button_filled.png",

		scripted_image = {
		commands = {
			{
				command = "circle_filled",
				input = {
					radius = 19,
					filling = "255 255 255 255"
				}
			}
			}
		}
	},


	action_button_border = {
		source_image_path = "official/gfx/generated/procedural/action_button_border.png",

		scripted_image = {
		commands = {
			{
				command = "circle_midpoint",
				input = {
					radius = 19,
					border_width = 1,		
					scale_alpha = false,
					constrain_angle = false,
					angle_start = 0,
					angle_end = 0,
					filling = "255 255 255 255"
				}
			}
			}
		}
	},

	circular_bar_medium = {
		source_image_path = "official/gfx/generated/procedural/circular_bar_medium.png",

		scripted_image = {
		commands = {
			{
				command = "circle_midpoint",
				input = {
					radius = 57,
					border_width = 1,		
					scale_alpha = false,
					constrain_angle = false,
					angle_start = 0,
					angle_end = 0,
					filling = "0 255 255 255"
				}
			},

			{
				command = "circle_midpoint",
				input = {
					radius = 55,
					border_width = 5,		
					scale_alpha = false,
					constrain_angle = false,
					angle_start = 0,
					angle_end = 0,
					filling = "255 255 255 255"
				}
			}
			}
		}
	},

	circular_bar_small = {
		source_image_path = "official/gfx/generated/procedural/circular_bar_small.png",

		scripted_image = {
		commands = {
			{
				command = "circle_midpoint",
				input = {
					radius = 17,
					border_width = 1,		
					scale_alpha = false,
					constrain_angle = false,
					angle_start = 0,
					angle_end = 0,
					filling = "0 255 255 255"
				}
			},

			{
				command = "circle_midpoint",
				input = {
					radius = 15,
					border_width = 5,		
					scale_alpha = false,
					constrain_angle = false,
					angle_start = 0,
					angle_end = 0,
					filling = "255 255 255 255"
				}
			}
		}
		}
	},

menu_game_logo = {
	source_image_path = 
 "official/gfx/menu_game_logo.png"
		},

wandering_cross = {
	source_image_path = 
 "official/gfx/wandering_cross.png"
		},

spell_border = {
	source_image_path = 
 "official/gfx/spell_border.png"
		},

cast_highlight = {
	source_image_path = 
 "official/gfx/cast_highlight.png"
		},

container_open_icon = {
	source_image_path = 
 "official/gfx/container_open_icon.png"
		},

container_closed_icon = {
	source_image_path = 
 "official/gfx/container_closed_icon.png"
		},

gui_cursor = {
	source_image_path = 
 "official/gfx/gui_cursor.png"
		},

gui_cursor_hover = {
	source_image_path = 
 "official/gfx/gui_cursor_hover.png"
		},

gui_cursor_add = {
	source_image_path = 
 "official/gfx/gui_cursor_add.png"
		},

gui_cursor_error = {
	source_image_path = 
 "official/gfx/gui_cursor_error.png"
		},

gui_cursor_minus = {
	source_image_path = 
 "official/gfx/gui_cursor_minus.png"
		},

blank = {
	source_image_path = 
 "official/gfx/blank.png"
		},

laser = {
	source_image_path = "official/gfx/laser.png",
 custom_neon_map_path = "official/gfx/laser_neon_map.png"

		},

laser_glow_edge = {
	source_image_path = 
 "official/gfx/laser_glow_edge.png"
		},

drop_hand_icon = {
	source_image_path = 
 "official/gfx/drop_hand_icon.png"
		},

secondary_hand_icon = {
	source_image_path = 
 "official/gfx/secondary_hand_icon.png"
		},

primary_hand_icon = {
	source_image_path = 
 "official/gfx/primary_hand_icon.png"
		},

shoulder_slot_icon = {
	source_image_path = 
 "official/gfx/shoulder_slot_icon.png"
		},

armor_slot_icon = {
	source_image_path = 
 "official/gfx/armor_slot_icon.png"
		},

chamber_slot_icon = {
	source_image_path = 
 "official/gfx/chamber_slot_icon.png"
		},

detachable_magazine_slot_icon = {
	source_image_path = 
 "official/gfx/detachable_magazine_slot_icon.png"
		},

gun_muzzle_slot_icon = {
	source_image_path = 
 "official/gfx/gun_muzzle_slot_icon.png"
	}

blink = {
	source_image_path = 
 "official/gfx/blink_%x.png"
	}
}