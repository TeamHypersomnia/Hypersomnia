-- local function acquire_sprite_filenames(property_names, end_number)
-- 	local out_filenames = {}
-- 	
-- 	for i=0, #property_names do
-- 		
-- 	end
-- end 
-- 
-- 
-- -- this table only serves as an example
-- local spritesheets_example = {
-- 	{ "torso" }, { "basic", "warrior", "ninja" }, { "rifle", "pistol", "barehands" }, { "shoot", "walk" }  
-- 	{ "legs"  }, { "basic", "ninja" }, { "walk", "run" }
-- }
-- 
-- 
-- local gameplay_spritesheets = {
-- 	{ { "torso" }, { "basic" }, { "barehands" }, { "walk" } },
-- 	{ { "legs" }, { "basic" }, { "walk" } }
-- }
-- 
-- 
-- local filenames =  {}
-- 
-- for i=0, #gameplay_spritesheets do
-- 	filenames = table.concatenate({ filenames, acquire_sprite_filenames(gameplay_spritesheets[i]) })
-- end



local out_filenames = get_all_files_in_directory("hypersomnia\\data\\gfx")



for k, v in pairs(out_filenames) do 
	--print (v) 
	for i, j in pairs(tokenize_string(v, "_.")) do
		print (j)
	end
end
 

return out_filenames