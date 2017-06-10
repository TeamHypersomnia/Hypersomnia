local serpent = require("scripts.serpent.src.serpent")

function table_to_string(table, name_of_table)
	return serpent.block(table, {name = name_of_table})
end