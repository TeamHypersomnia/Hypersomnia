function create_textures(atl, entries)
	for k,v in pairs(entries) do
		entries[k] = texture(v, atl)
	end
end

my_atlas = atlas()
collectgarbage("collect")

parent_root = "resources\\"

images = {
	crate = "crate.jpg",
	metal = "metal.jpg",
	
	background = "background.jpg",
	crosshair = "crosshair.png",
	
	blank = "blank.png",
	
	shotgun = "shotgun3.png",
	assault = "assault.png",
	fireaxe = "world_fireaxe.png",
	
	
	player_legs_1 = "player\\legs_1.png",
	player_legs_2 = "player\\legs_2.png",
	player_legs_3 = "player\\legs_3.png",
	player_legs_4 = "player\\legs_4.png",
	player_legs_5 = "player\\legs_5.png",

	player_walk_1 = 	"player\\walk_1.png",
	player_walk_2 = 	"player\\walk_2.png",
	player_walk_3 = 	"player\\walk_3.png",
	player_walk_4 = 	"player\\walk_4.png",
	player_walk_5 = 	"player\\walk_5.png",

	player_hands_1 = 	"player\\hands_1.png",
	player_hands_2 = 	"player\\hands_2.png",
	player_hands_3 = 	"player\\hands_3.png",
	player_hands_4 = 	"player\\hands_4.png",
	player_hands_5 = 	"player\\hands_5.png",
	
	player_hit_1   = 	"player\\hit_1.png",
	player_hit_2   = 	"player\\hit_2.png",
	player_hit_3   = 	"player\\hit_3.png",
	player_hit_4   = 	"player\\hit_4.png",
	player_hit_5   = 	"player\\hit_5.png",
	
	player_melee_walk_1 = 	    "player\\melee_walk_1.png",
	player_melee_walk_2 = 	    "player\\melee_walk_2.png",
	player_melee_walk_3 = 	    "player\\melee_walk_3.png",
	player_melee_walk_4 = 	    "player\\melee_walk_4.png",
	player_melee_walk_5 = 	    "player\\melee_walk_5.png",
	
	player_firearm_shot_1 = 	    "player\\firearm_shot_1.png",
	player_firearm_shot_2 = 	    "player\\firearm_shot_2.png",
	player_firearm_shot_3 = 	    "player\\firearm_shot_3.png",
	player_firearm_shot_4 = 	    "player\\firearm_shot_4.png",
	player_firearm_shot_5 = 	    "player\\firearm_shot_5.png",
	
	player_firearm_walk_1 = 	    "player\\firearm_walk_1.png",
	player_firearm_walk_2 = 	    "player\\firearm_walk_2.png",
	player_firearm_walk_3 = 	    "player\\firearm_walk_3.png",
	player_firearm_walk_4 = 	    "player\\firearm_walk_4.png",
	player_firearm_walk_5 = 	    "player\\firearm_walk_5.png",
	
	
	enemy_legs_1 = "player\\legs_1.png",
	enemy_legs_2 = "player\\legs_2.png",
	enemy_legs_3 = "player\\legs_3.png",
	enemy_legs_4 = "player\\legs_4.png",
	enemy_legs_5 = "player\\legs_5.png",

	enemy_walk_1 = 	"player\\walk_1.png",
	enemy_walk_2 = 	"player\\walk_2.png",
	enemy_walk_3 = 	"player\\walk_3.png",
	enemy_walk_4 = 	"player\\walk_4.png",
	enemy_walk_5 = 	"player\\walk_5.png",
	
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
	dead_back  = "dead_back.png"
}

for k, v in pairs(images) do
	images[k] = (parent_root .. v)
end

create_textures(my_atlas, images)
my_atlas:build()
my_atlas:nearest()