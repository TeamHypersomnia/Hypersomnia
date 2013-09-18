local my_atlas = atlas()
crate_sprite = sprite("Release\\resources\\crate.jpg", my_atlas)

my_atlas:build()

crate_sprite.size = vec2(100, 100)

local util = script()
local scene = script()

util:associate_filename("scripts\\entity_creation_util.lua")
scene:associate_filename("scripts\\sample_scene\\entities.lua")

util:call()
scene:call()
