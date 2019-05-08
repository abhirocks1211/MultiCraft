dofile(minetest.get_modpath("sfinv") .. "/api.lua")

sfinv.register_page("sfinv:inventory", {
	title = "Inventory",
	get = function(self, player, context)
		return sfinv.make_formspec(player, context, [[
				list[current_player;craft;1.75,0.5;3,3;]
				list[current_player;craftpreview;5.75,1.5;1,1;]
			]], true)
	end
})
