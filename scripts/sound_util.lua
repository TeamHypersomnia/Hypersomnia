global_music_table = {}

function create_music(filename)
	--local new_music = sfMusic()
	--new_music:openFromFile(filename)
	--new_music:setLoop(true)
	--
	--table.insert(global_music_table, new_music)
	--
	--return new_music
end

function create_sound(filename)
	--local new_sound = sfSoundBuffer()
	--new_sound:loadFromFile(filename)
	--return new_sound
end

function stop_all_music()
	for i=1, #global_music_table do
		global_music_table[i]:stop()
	end
end

global_sound_table = {}

function play_sound(what_buffer)
	--new_sound = sfSound()
	--new_sound:setBuffer(what_buffer)
	--new_sound:setVolume(100)
	--new_sound:play()
	--
	--local i = 1
	--while i <= #global_sound_table do
	--	if has_sound_stopped_playing(global_sound_table[i]) then
	--		table.remove(global_sound_table, i)
	--	else
	--		i = i + 1
	--	end
	--end
	--
	--table.insert(global_sound_table, new_sound)
end