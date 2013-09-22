local my_atlas = atlas()
crate_texture = texture("Release\\resources\\crate.jpg", my_atlas)

my_atlas:build()

local util = script()
local resource_util = script()
local scene = script()

util:associate_filename("scripts\\entity_creation_util.lua")
resource_util:associate_filename("scripts\\resource_creation_util.lua")
scene:associate_filename("scripts\\sample_scene\\entities.lua")

util:call()
resource_util:call()
scene:call()