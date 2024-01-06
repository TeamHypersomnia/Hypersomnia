return {
  extra_loadables = {
    enabled_generate_neon_map = {
      alpha_multiplier = 1,
      amplification = 90,
      light_colors = {
        "255 255 255 255",
        "255 0 255 255",
        "0 255 255 255",
        "255 0 0 255"
      },
      radius = {
        x = 80,
        y = 80
      },
      standard_deviation = 8
    },
    generate_desaturation = false
  },
  offsets = {
    gun = {
      bullet_spawn = {
        x = 0,
        y = -5
      },
      chamber = {
        pos = {
          x = 0,
          y = 0
        },
        rotation = 0
      },
      chamber_magazine = {
        pos = {
          x = 0,
          y = 0
        },
        rotation = 0
      },
      detachable_magazine = {
        pos = {
          x = -3,
          y = 8
        },
        rotation = 0
      },
      shell_spawn = {
        pos = {
          x = 10,
          y = -11
        },
        rotation = -90
      }
    },
    item = {
      akimbo_offset = {
        pos = {
          x = 10,
          y = 7
        },
        rotation = 0
      },
      back_anchor = {
        pos = {
          x = 0,
          y = 0
        },
        rotation = 0
      },
      beep_offset = {
        pos = {
          x = 0,
          y = 0
        },
        rotation = 0
      },
      hand_anchor = {
        pos = {
          x = -13 - 6,
          y = 2 - 2
        },
        rotation = 0
      },
      head_anchor = {
        pos = {
          x = -4,
          y = 0
        },
        rotation = -15
      },
      shoulder_anchor = {
        pos = {
          x = 0,
          y = 0
        },
        rotation = 0
      }
    },
    legs = {
      foot = {
        x = 0,
        y = 0
      }
    },
    non_standard_shape = {
      convex_partition = {
                        1,
                        2,
                        3,
                        4,
                        1,
                        1,
                        4,
                        5,
                        0,
                        1,
                        8,
                        9,
                        0,
                        7,
                        8,
                        7,
                        0,
                        5,
                        6,
                        7
      },
      source_polygon = {
{x=-22.0, y=-2.0}, {x=-36.0, y=-2.0}, {x=-44.0, y=4.0}, {x=-45.0, y=-3.0}, {x=-43.0, y=-8.0}, {x=44.0, y=-8.0}, {x=44.0, y=-2.0}, {x=-17.0, y=0.0}, {x=-21.0, y=9.0}, {x=-25.0, y=9.0}
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
      legs = {
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
      },
      secondary_shoulder = {
        pos = {
          x = 0,
          y = 0
        },
        rotation = 0
      },
      shoulder = {
        pos = {
          x = 0,
          y = 0
        },
        rotation = 0
      },
      strafe_facing_offset = 0
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