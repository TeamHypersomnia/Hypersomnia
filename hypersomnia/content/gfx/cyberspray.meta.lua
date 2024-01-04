return {
  extra_loadables = {
    enabled_generate_neon_map = {
      alpha_multiplier = 1,
      amplification = 60,
      light_colors = {
        "66 228 207 255",
        "228 20 206 255"
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
        y = -5
      },
      chamber = {
        pos = {
          x = -2,
          y = -7
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
          x = -2,
          y = 11
        },
        rotation = 0
      },
      rail = {
        pos = {
          x = 0,
          y = 0
        },
        rotation = 0
      },
      shell_spawn = {
        pos = {
          x = -5,
          y = -9
        },
        rotation = -90
      }
    },
    item = {
      akimbo_offset = {
        pos = {
          x = 23,
          y = -2
        },
        rotation = 11
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
          x = -4 - 23,
          y = 3 + 2 
        },
        rotation = -11
      },
      head_anchor = {
        pos = {
          x = -5,
          y = -3
        },
        rotation = -11
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
                        0,
                        1,
                        0,
                        3,
                        4,
                        5,
                        6,
                        0,
                        8,
                        0,
                        6,
                        7,
                        8
      },
      source_polygon = {
{x=-18.0, y=-2.0}, { x=-26.0, y=2.0}, { x=-26.5, y=-9.5}, { x=-21.0, y=-6.0}, { x=23.0, y=-8.0}, { x=23.0, y=-1.0}, { x=-11.0, y=-1.0}, { x=-11.0, y=10.0}, { x=-18.0, y=10.0}
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