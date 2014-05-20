return {
  version = "1.1",
  luaversion = "5.1",
  orientation = "orthogonal",
  width = 50,
  height = 50,
  tilewidth = 32,
  tileheight = 32,
  properties = {
    ["gameplay_textures"] = "hypersomnia\\data\\gfx",
    ["texture_directory"] = "hypersomnia\\data\\maps\\textures\\",
    ["type_library"] = "hypersomnia\\data\\maps\\basic_type_library"
  },
  tilesets = {},
  layers = {
    {
      type = "objectgroup",
      name = "Object Layer 1",
      visible = true,
      opacity = 1,
      properties = {},
      objects = {
        {
          name = "",
          type = "wall_wood",
          shape = "rectangle",
          x = 512,
          y = 544,
          width = 480,
          height = 64,
          visible = true,
          properties = {}
        },
        {
          name = "",
          type = "crate",
          shape = "rectangle",
          x = 992,
          y = 864,
          width = 160,
          height = 160,
          visible = true,
          properties = {}
        },
        {
          name = "",
          type = "teleport_position",
          shape = "rectangle",
          x = 736,
          y = 736,
          width = 96,
          height = 96,
          visible = true,
          properties = {}
        }
      }
    }
  }
}
