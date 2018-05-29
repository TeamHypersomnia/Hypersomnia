return {
  extra_loadables = {
    enabled_generate_neon_map = {
      alpha_multiplier = 1,
      amplification = 60,
      light_colors = {
        "0 255 255 255",
        "255 0 0 255",
        "255 0 255 255",
        "0 255 174 255",
        "255 0 228 255",
        "0 198 255 255"
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
        x = -15,
        y = 2
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
          x = 0,
          y = 0
        },
        rotation = 0
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
          x = 0,
          y = 0
        },
        rotation = 0
      },
      secondary_hand = {
        pos = {
          x = 0,
          y = 0
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