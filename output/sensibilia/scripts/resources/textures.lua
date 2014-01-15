function create_textures(atl, entries)
	for k,v in pairs(entries) do
		entries[k] = texture(v, atl)
	end
end

my_atlas = atlas()
collectgarbage("collect")

function add_roots(root_path, entries)
	local new_table = {}
	
	for k, v in pairs(entries) do
		new_table[k] = (root_path .. v)
	end
	
	return new_table
end

images = add_roots("sensibilia\\resources\\", {
	blank = "blank.png"
})

--character_filenames = {
--	walk_1 = "walk_1.png",
--	walk_2 = "walk_2.png",
--	walk_3 = "walk_3.png",
--	walk_4 = "walk_4.png",
--	walk_5 = "walk_5.png",
--	
--	hit_1   = "hit_1.png",
--	hit_2   = "hit_2.png",
--	hit_3   = "hit_3.png",
--	hit_4   = "hit_4.png",
--	hit_5   = "hit_5.png",
--	
--	melee_walk_1 = "melee_walk_1.png",
--	melee_walk_2 = "melee_walk_2.png",
--	melee_walk_3 = "melee_walk_3.png",
--	melee_walk_4 = "melee_walk_4.png",
--	melee_walk_5 = "melee_walk_5.png",
--	
--	firearm_shot_1 = "firearm_shot_1.png",
--	firearm_shot_2 = "firearm_shot_2.png",
--	firearm_shot_3 = "firearm_shot_3.png",
--	firearm_shot_4 = "firearm_shot_4.png",
--	firearm_shot_5 = "firearm_shot_5.png",
--	                 
--	firearm_walk_1 = "firearm_walk_1.png",
--	firearm_walk_2 = "firearm_walk_2.png",
--	firearm_walk_3 = "firearm_walk_3.png",
--	firearm_walk_4 = "firearm_walk_4.png",
--	firearm_walk_5 = "firearm_walk_5.png",
--	
--	legs_1 = "legs_1.png",
--	legs_2 = "legs_2.png",
--	legs_3 = "legs_3.png",
--	legs_4 = "legs_4.png",
--	legs_5 = "legs_5.png",
--	
--	dead_front = "dead_front.png"
--}
--
--enemy_images = add_roots("hp\\resources\\enemy\\", character_filenames)
--player_images = add_roots("hp\\resources\\player\\", archetyped(character_filenames, {
--	hands_1 = "hands_1.png",
--	hands_2 = "hands_2.png",
--	hands_3 = "hands_3.png",
--	hands_4 = "hands_4.png",
--	hands_5 = "hands_5.png"
--}))

create_textures(my_atlas, images)
--create_textures(my_atlas, player_images)
--create_textures(my_atlas, enemy_images)

my_atlas:build()
my_atlas:nearest()