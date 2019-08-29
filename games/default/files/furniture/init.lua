ts_furniture = {}

ts_furniture.sit = function(name, pos)
	local player = minetest.get_player_by_name(name)
	if player_api.player_attached[name] then
		player:set_eye_offset({x = 0, y = 0, z = 0}, {x = 0, y = 0, z = 0})
		player:set_physics_override(1, 1, 1)
		player_api.player_attached[name] = false
		player_api.set_animation(player, "stand", 30)
	else
		player:moveto(pos)
		player:set_eye_offset({x = 0, y = -7, z = 2}, {x = 0, y = 0, z = 0})
		player:set_physics_override(0, 0, 0)
		player_api.player_attached[name] = true
		player_api.set_animation(player, "sit", 30)
	end
end

ts_furniture.up = function(name, pos)
	local player = minetest.get_player_by_name(name)
	if player_api.player_attached[name] then
		player:set_eye_offset({x = 0, y = 0, z = 0}, {x = 0, y = 0, z = 0})
		player:set_physics_override(1, 1, 1)
		player_api.player_attached[name] = false
		player_api.set_animation(player, "stand", 30)
	end
end

local furnitures = {
	["chair"] = {
		description = "Chair",
		sitting = true,
		nodebox = {
			{ -0.3, -0.5,  0.2, -0.2,  0.5,  0.3 }, -- foot 1
			{  0.2, -0.5,  0.2,  0.3,  0.5,  0.3 }, -- foot 2
			{  0.2, -0.5, -0.3,  0.3, -0.1, -0.2 }, -- foot 3
			{ -0.3, -0.5, -0.3, -0.2, -0.1, -0.2 }, -- foot 4
			{ -0.3, -0.1, -0.3,  0.3,  0,    0.2 }, -- seating
			{ -0.2,  0.1,  0.25, 0.2,  0.4,  0.26}  -- conector 1-2
		},
		craft = function(recipe)
			return {
				{ "", "group:stick" },
				{ recipe, recipe },
				{ "group:stick", "group:stick" }
			}
		end
	},
	["table"] = {
		description = "Table",
		nodebox = {
			{ -0.4, -0.5, -0.4, -0.3, 0.4, -0.3 }, -- foot 1
			{  0.3, -0.5, -0.4,  0.4, 0.4, -0.3 }, -- foot 2
			{ -0.4, -0.5,  0.3, -0.3, 0.4,  0.4 }, -- foot 3
			{  0.3, -0.5,  0.3,  0.4, 0.4,  0.4 }, -- foot 4
			{ -0.5,  0.4, -0.5,  0.5, 0.5,  0.5 }  -- table top
		},
		craft = function(recipe)
			return {
				{ recipe, recipe, recipe },
				{ "group:stick", "", "group:stick" },
				{ "group:stick", "", "group:stick" }
			}
		end
	},
	["small_table"] = {
		description = "Small Table",
		nodebox = {
			{ -0.4, -0.5, -0.4, -0.3, 0.1, -0.3 }, -- foot 1
			{  0.3, -0.5, -0.4,  0.4, 0.1, -0.3 }, -- foot 2
			{ -0.4, -0.5,  0.3, -0.3, 0.1,  0.4 }, -- foot 3
			{  0.3, -0.5,  0.3,  0.4, 0.1,  0.4 }, -- foot 4
			{ -0.5,  0.1, -0.5,  0.5, 0.2,  0.5 }, -- table top
		},
		craft = function(recipe)
			return {
				{ recipe, recipe, recipe },
				{ "group:stick", "", "group:stick" }
			}
		end
	},
	["tiny_table"] = {
		description = "Tiny Table",
		nodebox = {
			{ -0.5, -0.1, -0.5,  0.5,  0,   0.5 }, -- table top
			{ -0.4, -0.5, -0.5, -0.3, -0.1, 0.5 }, -- foot 1
			{  0.3, -0.5, -0.5,  0.4, -0.1, 0.5 }, -- foot 2
		},
		craft = function(recipe)
			local bench_name = "ts_furniture:" .. recipe:gsub(":", "_") .. "_bench"
			return {
				{ bench_name, bench_name }
			}
		end
	},
	["bench"] = {
		description = "Bench",
		sitting = true,
		nodebox = {
			{ -0.5, -0.1, 0,  0.5,  0,   0.5 }, -- seating
			{ -0.4, -0.5, 0, -0.3, -0.1, 0.5 }, -- foot 1
			{  0.3, -0.5, 0,  0.4, -0.1, 0.5 }, -- foot 2
		},
		craft = function(recipe)
			return {
				{ recipe, recipe },
				{ "group:stick", "group:stick" },
				{ "group:stick", "group:stick" }
			}
		end
	},
	["bedsidetable"] = {
		description = "Bedside Table",
		nodebox = {
			{ -0.5, -0.5, -0.5, -0.4, 0.5,  0.5 },
			{  0.5, -0.5, -0.5,  0.4, 0.5,  0.5 },
			{ -0.5,  0.4, -0.5,  0.5, 0.5,  0.5 },
			{ -0.5,  0,   -0.5,  0.5, 0.1,  0.5 },
			{ -0.5, -0.5,  0.5,  0.5, 0.5,  0.4 }
		},
		craft = function(recipe)
			return {
				{ recipe, recipe },
				{ "group:stick", "group:stick" }
			}
		end
	},
	["endtable"] = {
		description = "End Table",
		nodebox = {
			{ -0.5, -0.5, -0.5, -0.4,  0.5, -0.4 },
			{ -0.5, -0.5,  0.5, -0.4,  0.5,  0.4 },
			{  0.5, -0.5, -0.5,  0.4,  0.5, -0.4 },
			{  0.5, -0.5,  0.5,  0.4,  0.5,  0.4 },
			{  0.5,  0.4,  0.5, -0.5,  0.5, -0.5 },
			{  0.5, -0.3,  0.5, -0.5, -0.2, -0.5 }
		},
		craft = function(recipe)
			return {
				{ recipe, "", recipe },
				{ "group:stick", "", "group:stick" },
				{ recipe, "", recipe }
			}
		end
	},
	["coffeetable"] = {
		description = "Coffee Table",
		nodebox = {
			{ -0.5, -0.5, -0.5, -0.4,  0,   -0.4 },
			{ -0.5, -0.5,  0.5, -0.4,  0,    0.4 },
			{  0.5, -0.5, -0.5,  0.4,  0,   -0.4 },
			{  0.5, -0.5,  0.5,  0.4,  0,    0.4 },
			{  0.5,  0.1,  0.5, -0.5,  0,   -0.5 },
			{  0.5, -0.3,  0.5, -0.5, -0.4, -0.5 }
		},
		craft = function(recipe)
			return {
				{ recipe, "", recipe },
				{ "group:stick", "", "group:stick" }
			}
		end
	}
}

local ignore_groups = {
	["wood"] = true,
	["stone"] = true
}

function ts_furniture.register_furniture(recipe, description, tiles, fpairs)
	local recipe_def = minetest.registered_items[recipe]
	if not recipe_def then
		return
	end

	local groups = {falling_node = 1}

	for k, v in pairs(recipe_def.groups) do
		if not ignore_groups[k] then
			groups[k] = v
		end
	end

	for fpairs, def in pairs(fpairs) do
		local node_name = "ts_furniture:" .. recipe:gsub(":", "_") .. "_" .. fpairs

		if def.sitting then
			def.on_rightclick = function(pos, node, player, itemstack, pointed_thing)
				ts_furniture.sit(player:get_player_name(), pos)
			end
			def.on_punch = function(pos, node, player, itemstack, pointed_thing)
				ts_furniture.up(player:get_player_name(), pos)
			end
		end

		def.drawtype = def.drawtype or "nodebox"

		minetest.register_node(":" .. node_name, {
			description = Sl(description) .. " " .. Sl(def.description),
			drawtype = def.drawtype,
			mesh = def.mesh,
			paramtype = "light",
			paramtype2 = "facedir",
			sunlight_propagates = true,
			tiles = { tiles },
			groups = groups,
			node_box = {
				type = "fixed",
				fixed = def.nodebox
			},
			on_rightclick = def.on_rightclick,
			on_punch = def.on_punch
		})

		minetest.register_craft({
			output = node_name,
			recipe = def.craft(recipe)
		})
	end
end

ts_furniture.register_furniture("default:birch_wood", "Birch", "default_birch_wood.png", furnitures)
ts_furniture.register_furniture("default:pine_wood", "Pine", "default_pine_wood.png", furnitures)
ts_furniture.register_furniture("default:acacia_wood", "Acacia", "default_acacia_wood.png", furnitures)
ts_furniture.register_furniture("default:wood", "Wooden", "default_wood.png", furnitures)
ts_furniture.register_furniture("default:junglewood", "Jungle Wood", "default_junglewood.png", furnitures)
