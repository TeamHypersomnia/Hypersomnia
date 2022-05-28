local serpent = require("detail.serpent.src.serpent")
local ltdiff = require("detail.ltdiff")

function table_to_string(table, name_of_table)
	return serpent.block(table, { name = name_of_table, comment = false })
end

function patch_table(table, patch)
	return ltdiff.patch(table, patch)
end

function table_diff_to_string(table, other, name_of_table)
	local diff = ltdiff.diff(table, other)
	return serpent.block(diff, { name = name_of_table, comment = false })
end