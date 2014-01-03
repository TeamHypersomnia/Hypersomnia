function create_textures(atl, entries)
	for k,v in pairs(entries) do
		entries[k] = texture(v, atl)
	end
end

my_atlas = atlas()
collectgarbage("collect")

function add_roots(entries, root_path)
	local new_table = {}
	
	for k, v in pairs(entries) do
		new_table[k] = (parent_root .. v)
	end
	
	return new_table
end

images = add_roots("resources\\", {
	crate = "crate.jpg",
	metal = "metal.jpg",
	
	background = "background.jpg",
	crosshair = "crosshair.png",
	
	blank = "blank.png",
	
	shotgun = "shotgun3.png",
	assault = "assault.png",
	fireaxe = "world_fireaxe.png",
	
    floor1 = "floor1.jpg",
	floor2 = "floor2.jpg",
	floor3 = "floor3.jpg",
	floor4 = "floor4.jpg",
	floor5 = "floor5.jpg",
	floor6 = "floor6.jpg",
	wall1 = "wall1.jpg",
	wall2 = "wall2.jpg",
	
	wood_table = "table.png",
	
	
	
	enemy_legs_1 = "enemy\\legs_1.png",
	enemy_legs_2 = "enemy\\legs_2.png",
	enemy_legs_3 = "enemy\\legs_3.png",
	enemy_legs_4 = "enemy\\legs_4.png",
	enemy_legs_5 = "enemy\\legs_5.png",

	enemy_walk_1 = 	"enemy\\walk_1.png",
	enemy_walk_2 = 	"enemy\\walk_2.png",
	enemy_walk_3 = 	"enemy\\walk_3.png",
	enemy_walk_4 = 	"enemy\\walk_4.png",
	enemy_walk_5 = 	"enemy\\walk_5.png",
	
	enemy_hit_1   = 	"enemy\\hit_1.png",
	enemy_hit_2   = 	"enemy\\hit_2.png",
	enemy_hit_3   = 	"enemy\\hit_3.png",
	enemy_hit_4   = 	"enemy\\hit_4.png",
	enemy_hit_5   = 	"enemy\\hit_5.png",
	
	enemy_melee_walk_1 = 	    "enemy\\melee_walk_1.png",
	enemy_melee_walk_2 = 	    "enemy\\melee_walk_2.png",
	enemy_melee_walk_3 = 	    "enemy\\melee_walk_3.png",
	enemy_melee_walk_4 = 	    "enemy\\melee_walk_4.png",
	enemy_melee_walk_5 = 	    "enemy\\melee_walk_5.png",
	
	enemy_firearm_shot_1 = 	    "enemy\\firearm_shot_1.png",
	enemy_firearm_shot_2 = 	    "enemy\\firearm_shot_2.png",
	enemy_firearm_shot_3 = 	    "enemy\\firearm_shot_3.png",
	enemy_firearm_shot_4 = 	    "enemy\\firearm_shot_4.png",
	enemy_firearm_shot_5 = 	    "enemy\\firearm_shot_5.png",
	
	enemy_firearm_walk_1 = 	    "enemy\\firearm_walk_1.png",
	enemy_firearm_walk_2 = 	    "enemy\\firearm_walk_2.png",
	enemy_firearm_walk_3 = 	    "enemy\\firearm_walk_3.png",
	enemy_firearm_walk_4 = 	    "enemy\\firearm_walk_4.png",
	enemy_firearm_walk_5 = 	    "enemy\\firearm_walk_5.png",
	
	head_1 = "enemy\\head_1.png",
	head_2 = "enemy\\head_2.png",
	head_3 = "enemy\\head_3.png",
	head_4 = "enemy\\head_4.png",
	head_5 = "enemy\\head_5.png",
	head_6 = "enemy\\head_6.png",
	head_7 = "enemy\\head_7.png",
	head_8 = "enemy\\head_8.png",
	head_9 = "enemy\\head_9.png",
	head_10 = "enemy\\head_10.png",
	head_11 = "enemy\\head_11.png",
	head_12 = "enemy\\head_12.png",
	head_13 = "enemy\\head_13.png",
	head_14 = "enemy\\head_14.png",
	head_15 = "enemy\\head_15.png",
	
	head_walk = "player\\head_walk.png",
	head_gun = "player\\head_gun.png",
	head_shot = "player\\head_shot.png",
	head_over = "player\\head_over.png",
	
	shotgun_wielded = "shotgun_wielded.png",
	assault_wielded = "assault_wielded.png",
	
	crate = "crate.jpg",

	bullet = "bullet.png",
	
	
	
	--legs_1 = "legs_1.png",
	--legs_2 = "legs_2.png",
	--legs_3 = "legs_3.png",
	--legs_4 = "legs_4.png",
	--legs_5 = "legs_5.png",
	--legs_6 = "legs_6.png",
	--legs_7 = "legs_7.png",
	--legs_8 = "legs_8.png",
	--legs_9 = "legs_9.png",
	--legs_10 = "legs_10.png",
	--
	--enemy_shotgun_1 = "enemy_shotgun_1.png",
	--enemy_shotgun_2 = "enemy_shotgun_2.png",
	--enemy_shotgun_3 = "enemy_shotgun_3.png",
	--enemy_shotgun_4 = "enemy_shotgun_4.png",
	--enemy_shotgun_5 = "enemy_shotgun_5.png",
	--
	--enemy_shotgun_shot_1 = "enemy_shotgun_shot_1.png",
	--enemy_shotgun_shot_2 = "enemy_shotgun_shot_2.png",
	--enemy_shotgun_shot_3 = "enemy_shotgun_shot_3.png",
	--enemy_shotgun_shot_4 = "enemy_shotgun_shot_4.png",
	--enemy_shotgun_shot_5 = "enemy_shotgun_shot_5.png",

	piece_1 = "piece_1.png",
	piece_2 = "piece_2.png",
	piece_3 = "piece_3.png",
	piece_4 = "piece_4.png",
	piece_5 = "piece_5.png",
	piece_6 = "piece_6.png",
	piece_7 = "piece_7.png",
	piece_8 = "piece_8.png",

	smoke_particle = "smoke_particle.png",

	blood_1 = "blood_1.png",
	blood_2 = "blood_2.png",
	blood_3 = "blood_3.png",
	blood_4 = "blood_4.png",
	blood_5 = "blood_5.png",

	dead_front = "dead_front.png",
	dead_front_player = "player\\dead_front.png",
	dead_back  = "dead_back.png"
})

character_filenames = {
	walk_1 = "walk_1.png",
	walk_2 = "walk_2.png",
	walk_3 = "walk_3.png",
	walk_4 = "walk_4.png",
	walk_5 = "walk_5.png",

	hands_1 = "hands_1.png",
	hands_2 = "hands_2.png",
	hands_3 = "hands_3.png",
	hands_4 = "hands_4.png",
	hands_5 = "hands_5.png",
	
	hit_1   = "hit_1.png",
	hit_2   = "hit_2.png",
	hit_3   = "hit_3.png",
	hit_4   = "hit_4.png",
	hit_5   = "hit_5.png",
	
	melee_walk_1 = "melee_walk_1.png",
	melee_walk_2 = "melee_walk_2.png",
	melee_walk_3 = "melee_walk_3.png",
	melee_walk_4 = "melee_walk_4.png",
	melee_walk_5 = "melee_walk_5.png",
	
	firearm_shot_1 = "firearm_shot_1.png",
	firearm_shot_2 = "firearm_shot_2.png",
	firearm_shot_3 = "firearm_shot_3.png",
	firearm_shot_4 = "firearm_shot_4.png",
	firearm_shot_5 = "firearm_shot_5.png",
	                 
	firearm_walk_1 = "firearm_walk_1.png",
	firearm_walk_2 = "firearm_walk_2.png",
	firearm_walk_3 = "firearm_walk_3.png",
	firearm_walk_4 = "firearm_walk_4.png",
	firearm_walk_5 = "firearm_walk_5.png",
}

enemy_images = add_roots("resources\\enemy\\", character_filenames)
player_images = add_roots("resources\\player\\", archetyped(character_filenames, {
	legs_1 = "legs_1.png",
	legs_2 = "legs_2.png",
	legs_3 = "legs_3.png",
	legs_4 = "legs_4.png",
	legs_5 = "legs_5.png",
}))

create_textures(my_atlas, images)
create_textures(my_atlas, player_images)
create_textures(my_atlas, enemy_images)

my_atlas:build()
my_atlas:nearest()