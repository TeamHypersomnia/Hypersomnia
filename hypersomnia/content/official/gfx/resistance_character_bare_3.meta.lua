return {
  extra_loadables = {
    enabled_generate_neon_map = {
      alpha_multiplier = 1,
      amplification = 60,
      light_colors = {
        "255 180 0 255"
      },
      radius = {
        x = 80,
        y = 80
      },
      standard_deviation = 6
    },
    generate_desaturation = false
  },
  offsets = {
    gun = {
      bullet_spawn = {
        x = 0,
        y = 0
      },
      detachable_magazine = {
        pos = {
          x = 0,
          y = 0
        },
        rotation = 0
      }
    },
    item = {
      attachment_anchor = {
        x = 0,
        y = 0
      },
      back_anchor = {
        x = 0,
        y = 0
      },
      hand_anchor = {
        x = 0,
        y = 0
      }
    },
    legs = {
      foot = {
        x = 0,
        y = 0
      }
    },
    torso = {
      back = {
        pos = {
          x = -18,
          y = 0
        },
        rotation = -2
      },
      head = {
        pos = {
          x = 0,
          y = 0
        },
        rotation = 0
      },
      primary_hand = {
        pos = {
          x = 17,
          y = 20
        },
        rotation = 0
      },
      secondary_hand = {
        pos = {
          x = 14,
          y = -19
        },
        rotation = 0
      }
    }
  },
  usage_as_button = {
    bbox_expander = {
      x = 0,
      y = 0
    },
    flip = {
      horizontally = false,
      vertically = false
    }
  }
}