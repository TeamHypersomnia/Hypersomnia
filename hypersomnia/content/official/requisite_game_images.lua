{
	hotbar_button = {
		source_image_path = "official/gfx/generated/hotbar_button",

		button_with_corners = {

		}
	},

	attachment_circle_filled = {
		source_image_path = "official/gfx/generated/attachment_circle_filled",

		procedural_image = {
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
		source_image_path = "official/gfx/generated/attachment_circle_border",

		procedural_image = {
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
		source_image_path = "official/gfx/generated/action_button_filled",

		procedural_image = {
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
		source_image_path = "official/gfx/generated/action_button_border",

		procedural_image = {
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
		source_image_path = "official/gfx/generated/circular_bar_medium",

		procedural_image = {
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
		source_image_path = "official/gfx/generated/circular_bar_small",

		procedural_image = {
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
	}
}