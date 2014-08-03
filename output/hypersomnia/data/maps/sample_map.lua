return {
  version = "1.1",
  luaversion = "5.1",
  orientation = "orthogonal",
  width = 50,
  height = 50,
  tilewidth = 32,
  tileheight = 32,
  properties = {
    ["gameplay_textures"] = "..\\gfx\\",
    ["texture_directory"] = "textures\\",
    ["type_library"] = "basic_type_library.lua"
  },
  tilesets = {},
  layers = {
    {
      type = "objectgroup",
      name = "objects",
      visible = true,
      opacity = 1,
      properties = {},
      objects = {
        {
          name = "",
          type = "wall_wood",
          shape = "rectangle",
          x = 576,
          y = 544,
          width = 320,
          height = 64,
          visible = true,
          properties = {}
        },
        {
          name = "",
          type = "wall_wood",
          shape = "rectangle",
          x = 736,
          y = 128,
          width = 96,
          height = 96,
          visible = true,
          properties = {}
        },
        {
          name = "",
          type = "wall_wood",
          shape = "polygon",
          x = 480,
          y = 896,
          width = 0,
          height = 0,
          visible = true,
          polygon = {
            { x = 0, y = 0 },
            { x = -160, y = -32 },
            { x = -320, y = 128 },
            { x = -96, y = 320 }
          },
          properties = {}
        },
        {
          name = "",
          type = "wall_wood",
          shape = "polygon",
          x = 1248,
          y = 288,
          width = 0,
          height = 0,
          visible = true,
          polygon = {
            { x = 0, y = 0 },
            { x = 64, y = 224 },
            { x = -160, y = 256 },
            { x = -160, y = -160 }
          },
          properties = {}
        },
        {
          name = "",
          type = "wall_wood",
          shape = "polygon",
          x = 800,
          y = 992,
          width = 0,
          height = 0,
          visible = true,
          polygon = {
            { x = 0, y = 0 },
            { x = -288, y = 320 },
            { x = -384, y = 512 },
            { x = -192, y = 576 },
            { x = 224, y = 544 },
            { x = 608, y = 576 },
            { x = 800, y = 480 },
            { x = 736, y = 192 },
            { x = 736, y = -128 },
            { x = 608, y = -256 },
            { x = 480, y = -352 },
            { x = 352, y = -320 },
            { x = 448, y = -224 },
            { x = 640, y = -96 },
            { x = 640, y = 160 },
            { x = 544, y = 288 },
            { x = 672, y = 416 },
            { x = 544, y = 480 },
            { x = 320, y = 384 },
            { x = 224, y = 480 },
            { x = 0, y = 480 },
            { x = -160, y = 512 },
            { x = -288, y = 416 },
            { x = 0, y = 192 }
          },
          properties = {}
        }
      }
    },
    {
      type = "objectgroup",
      name = "ground",
      visible = true,
      opacity = 1,
      properties = {},
      objects = {
        {
          name = "",
          type = "ground",
          shape = "rectangle",
          x = -320,
          y = -128,
          width = 2208,
          height = 1888,
          visible = true,
          properties = {
            ["texture"] = "snow.jpg"
          }
        },
        {
          name = "",
          type = "ground",
          shape = "rectangle",
          x = 1888,
          y = -128,
          width = 2208,
          height = 1888,
          visible = true,
          properties = {
            ["texture"] = "snow.jpg"
          }
        },
        {
          name = "",
          type = "ground",
          shape = "rectangle",
          x = 1888,
          y = -2016,
          width = 2208,
          height = 1888,
          visible = true,
          properties = {
            ["texture"] = "snow.jpg"
          }
        },
        {
          name = "",
          type = "ground",
          shape = "rectangle",
          x = -320,
          y = -2016,
          width = 2208,
          height = 1888,
          visible = true,
          properties = {
            ["texture"] = "snow.jpg"
          }
        },
        {
          name = "",
          type = "ground",
          shape = "rectangle",
          x = -2528,
          y = -2016,
          width = 2208,
          height = 1888,
          visible = true,
          properties = {
            ["texture"] = "snow.jpg"
          }
        },
        {
          name = "",
          type = "ground",
          shape = "rectangle",
          x = -2528,
          y = -128,
          width = 2208,
          height = 1888,
          visible = true,
          properties = {
            ["texture"] = "snow.jpg"
          }
        },
        {
          name = "",
          type = "ground",
          shape = "rectangle",
          x = -2528,
          y = 1760,
          width = 2208,
          height = 1888,
          visible = true,
          properties = {
            ["texture"] = "snow.jpg"
          }
        },
        {
          name = "",
          type = "ground",
          shape = "rectangle",
          x = 1888,
          y = 1760,
          width = 2208,
          height = 1888,
          visible = true,
          properties = {
            ["texture"] = "snow.jpg"
          }
        },
        {
          name = "",
          type = "ground",
          shape = "rectangle",
          x = -320,
          y = 1760,
          width = 2208,
          height = 1888,
          visible = true,
          properties = {
            ["texture"] = "snow.jpg"
          }
        }
      }
    },
    {
      type = "objectgroup",
      name = "teleporters",
      visible = true,
      opacity = 1,
      properties = {},
      objects = {
        {
          name = "",
          type = "teleport_position",
          shape = "rectangle",
          x = 192,
          y = 160,
          width = 96,
          height = 96,
          visible = true,
          properties = {}
        },
        {
          name = "",
          type = "teleport_position",
          shape = "rectangle",
          x = 672,
          y = 1344,
          width = 96,
          height = 96,
          visible = true,
          properties = {}
        },
        {
          name = "",
          type = "teleport_position",
          shape = "rectangle",
          x = 1280,
          y = 1312,
          width = 96,
          height = 96,
          visible = true,
          properties = {}
        },
        {
          name = "",
          type = "teleport_position",
          shape = "rectangle",
          x = 1408,
          y = 608,
          width = 96,
          height = 96,
          visible = true,
          properties = {}
        },
        {
          name = "",
          type = "teleport_position",
          shape = "rectangle",
          x = 128,
          y = 1152,
          width = 96,
          height = 96,
          visible = true,
          properties = {}
        },
        {
          name = "",
          type = "teleport_position",
          shape = "rectangle",
          x = 320,
          y = 1344,
          width = 96,
          height = 96,
          visible = true,
          properties = {}
        },
        {
          name = "",
          type = "teleport_position",
          shape = "rectangle",
          x = 832,
          y = 1056,
          width = 96,
          height = 96,
          visible = true,
          properties = {}
        },
        {
          name = "",
          type = "teleport_position",
          shape = "rectangle",
          x = 704,
          y = 640,
          width = 96,
          height = 96,
          visible = true,
          properties = {}
        },
        {
          name = "",
          type = "teleport_position",
          shape = "rectangle",
          x = 1312,
          y = 160,
          width = 96,
          height = 96,
          visible = true,
          properties = {}
        }
      }
    }
  }
}
