function create_textures(atl, entries)
	for k,v in pairs(entries) do
		entries[k] = texture(v, atl)
	end
end

my_atlas = atlas()
collectgarbage("collect")

parent_root = "Release\\resources\\"

images = {
	crate = "crate.jpg",
	metal = "metal.jpg",
	
	background = "background.jpg",
	crosshair = "crosshair.png",
	
	blank = "blank.png",
	
	shotgun = "shotgun.png",
	m4a1 = "m4a1.png",
	fireaxe = "fireaxe.png",
	
	legs_1 = "legs_1.png",
	legs_2 = "legs_2.png",
	legs_3 = "legs_3.png",
	legs_4 = "legs_4.png",
	legs_5 = "legs_5.png",
	legs_6 = "legs_6.png",
	legs_7 = "legs_7.png",
	legs_8 = "legs_8.png",
	legs_9 = "legs_9.png",
	legs_10 = "legs_10.png",

	player_1 = "player_1.png",
	player_2 = "player_2.png",
	player_3 = "player_3.png",
	player_4 = "player_4.png",
	player_5 = "player_5.png",
	player_6 = "player_6.png",
	player_7 = "player_7.png",
	player_8 = "player_8.png",
	player_9 = "player_9.png",
	player_10 = "player_10.png",

	crate = "crate.jpg",

	player_shotgun_1 = "player_shotgun_1.png",
	player_shotgun_2 = "player_shotgun_2.png",
	player_shotgun_3 = "player_shotgun_3.png",
	player_shotgun_4 = "player_shotgun_4.png",
	player_shotgun_5 = "player_shotgun_5.png",

	bullet = "bullet.png",

	player_shotgun_shot_1 = "player_shotgun_shot_1.png",
	player_shotgun_shot_2 = "player_shotgun_shot_2.png",
	player_shotgun_shot_3 = "player_shotgun_shot_3.png",
	player_shotgun_shot_4 = "player_shotgun_shot_4.png",
	player_shotgun_shot_5 = "player_shotgun_shot_5.png",

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