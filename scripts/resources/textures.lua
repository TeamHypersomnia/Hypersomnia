function create_textures(atl, entries)
	for k,v in pairs(entries) do
		entries[k] = texture(v, atl)
	end
end

my_atlas = atlas()
collectgarbage("collect")

images = {
	crate = "Release\\resources\\crate.jpg",
	metal = "Release\\resources\\metal.jpg",
	
	background = "Release\\resources\\background.jpg",
	enemy = "Release\\resources\\enemy.png",
	crosshair = "Release\\resources\\crosshair.png",
	rifle = "Release\\resources\\rifle.png",
	
	legs_1 = "Release\\resources\\legs_1.png",
	legs_2 = "Release\\resources\\legs_2.png",
	legs_3 = "Release\\resources\\legs_3.png",
	legs_4 = "Release\\resources\\legs_4.png",
	legs_5 = "Release\\resources\\legs_5.png",
	legs_6 = "Release\\resources\\legs_6.png",
	legs_7 = "Release\\resources\\legs_7.png",
	legs_8 = "Release\\resources\\legs_8.png",
	legs_9 = "Release\\resources\\legs_9.png",
	legs_10 = "Release\\resources\\legs_10.png",

	player_1 = "Release\\resources\\player_1.png",
	player_2 = "Release\\resources\\player_2.png",
	player_3 = "Release\\resources\\player_3.png",
	player_4 = "Release\\resources\\player_4.png",
	player_5 = "Release\\resources\\player_5.png",
	player_6 = "Release\\resources\\player_6.png",
	player_7 = "Release\\resources\\player_7.png",
	player_8 = "Release\\resources\\player_8.png",
	player_9 = "Release\\resources\\player_9.png",
	player_10 = "Release\\resources\\player_10.png",

	crate = "Release\\resources\\crate.jpg",

	player_shotgun_1 = "Release\\resources\\player_shotgun_1.png",
	player_shotgun_2 = "Release\\resources\\player_shotgun_2.png",
	player_shotgun_3 = "Release\\resources\\player_shotgun_3.png",
	player_shotgun_4 = "Release\\resources\\player_shotgun_4.png",
	player_shotgun_5 = "Release\\resources\\player_shotgun_5.png",

	bullet = "Release\\resources\\bullet.png",

	player_shotgun_shot_1 = "Release\\resources\\player_shotgun_shot_1.png",
	player_shotgun_shot_2 = "Release\\resources\\player_shotgun_shot_2.png",
	player_shotgun_shot_3 = "Release\\resources\\player_shotgun_shot_3.png",
	player_shotgun_shot_4 = "Release\\resources\\player_shotgun_shot_4.png",
	player_shotgun_shot_5 = "Release\\resources\\player_shotgun_shot_5.png",

	piece_1 = "Release\\resources\\piece_1.png",
	piece_2 = "Release\\resources\\piece_2.png",
	piece_3 = "Release\\resources\\piece_3.png",
	piece_4 = "Release\\resources\\piece_4.png",
	piece_5 = "Release\\resources\\piece_5.png",
	piece_6 = "Release\\resources\\piece_6.png",
	piece_7 = "Release\\resources\\piece_7.png",
	piece_8 = "Release\\resources\\piece_8.png",

	smoke_particle = "Release\\resources\\smoke_particle.png",

	blood_1 = "Release\\resources\\blood_1.png",
	blood_2 = "Release\\resources\\blood_2.png",
	blood_3 = "Release\\resources\\blood_3.png",
	blood_4 = "Release\\resources\\blood_4.png",
	blood_5 = "Release\\resources\\blood_5.png",

	dead_front = "Release\\resources\\dead_front.png",
	dead_back  = "Release\\resources\\dead_back.png"
}

create_textures(my_atlas, images)
my_atlas:build()