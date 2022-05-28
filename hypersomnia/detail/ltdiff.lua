--[[
Copyright © 2012-14 Martin Felis <martin@fysx.org>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
]]--

local ltdiff = {}

function deepcompare(t1,t2,ignore_mt)
	local ty1 = type(t1)
	local ty2 = type(t2)
	if ty1 ~= ty2 then return false end
	-- non-table types can be directly compared
	if ty1 ~= 'table' and ty2 ~= 'table' then return t1 == t2 end
	-- as well as tables which have the metamethod __eq
	local mt = getmetatable(t1)
	if not ignore_mt and mt and mt.__eq then return t1 == t2 end
	for k1,v1 in pairs(t1) do
		local v2 = t2[k1]
		if v2 == nil or not deepcompare(v1,v2) then return false end
	end
	for k2,v2 in pairs(t2) do
		local v1 = t1[k2]
		if v1 == nil or not deepcompare(v1,v2) then return false end
	end
	return true
end

local function isArray(t)
  local i = 0
  for _ in pairs(t) do
    i = i + 1
    if t[i] == nil then return false end
  end
  return true
end

local function table_diff (A, B)
	local diff = { del = {} }

	for k,v in pairs(A) do
		if type(A[k]) == "function" or type(A[k]) == "userdata" then
			error ("table_diff only supports diffs of tables!")
		elseif B[k] ~= nil and type(A[k]) == "table" and type(B[k]) == "table" then
			if isArray(A[k]) then
				if not deepcompare(A[k], B[k]) then
					diff[k] = B[k]
				end
			else
				diff[k] = table_diff(A[k], B[k])

				if next(diff[k]) == nil then
					diff[k] = nil
				end
			end
		elseif B[k] == nil then
			diff.del[#(diff.del) + 1] = k
		elseif B[k] ~= v then
			diff[k] = B[k]
		end
	end

	for k,v in pairs(B) do
		if type(B[k]) == "function" or type(B[k]) == "userdata" then
			error ("table_diff only supports diffs of tables!")
		elseif diff[k] ~= nil then
			-- skip	
		elseif A[k] ~= nil and type(A[k]) == "table" and type(B[k]) == "table" then
			diff[k] = table_diff(B[k], A[k])

			if next(diff[k]) == nil then
				diff[k] = nil
			end
		elseif B[k] ~= A[k] then
			diff[k] = v
		end
	end

	if next(diff.del) == nil then
		diff.del = nil
	end

	return diff
end

local function table_patch (A, diff)
	if diff.del ~= nil then
		for k,v in pairs(diff.del) do
			A[v] = nil
		end
	end

	for k,v in pairs(diff) do
		if k ~= "del" then
			if type(A[k]) == "table" and type(v) == "table" then
				if isArray(A[k]) then
					A[k] = v
				else
					A[k] = table_patch (A[k], v)
				end
			else
				A[k] = v
			end
		end
	end

	return A
end

ltdiff.diff = table_diff
ltdiff.patch = table_patch

return ltdiff
