-- Kitten by Jordach / BFD
mobs:register_mob("mobs_animal:kitten", {
	stepheight = 0.6,
	type = "animal",
	passive = true,
	hp_min = 8,
	hp_max = 10,
	armor = 100,
	collisionbox = {-0.3, -0.3, -0.3, 0.3, 0.1, 0.3},
	visual = "mesh",
	visual_size = {x = 0.5, y = 0.5},
	mesh = "mobs_kitten.b3d",
	textures = {
		{"mobs_kitten_striped.png"},
		{"mobs_kitten_splotchy.png"},
		{"mobs_kitten_ginger.png"},
		{"mobs_kitten_sandy.png"},
	},
	makes_footstep_sound = false,
	sounds = {
		random = "mobs_kitten",
	},
	walk_velocity = 0.6,
	walk_chance = 15,
	run_velocity = 2,
	runaway = true,
	jump = true,
	jump_height = 5,
	drops = {
		{name = "farming:string", chance = 1, min = 1, max = 1}
	},
	water_damage = 1,
	lava_damage = 5,
	fall_damage = 2,
	fear_height = 3,
	animation = {
		speed_normal = 42,
		stand_start = 97,
		stand_end = 192,
		walk_start = 0,
		walk_end = 96,
		stoodup_start = 0,
		stoodup_end = 0,
	},
	follow = {"mobs_animal:rat", "default:fish_raw"},
	view_range = 8,
	on_rightclick = function(self, clicker)

		if mobs:feed_tame(self, clicker, 4, true, true) then return end
		if mobs:protect(self, clicker) then return end
		--if mobs:capture_mob(self, clicker, 50, 50, 90, false, nil) then return end
		end
})

mobs:spawn({
	name = "mobs_animal:kitten",
	nodes = {"default:dirt", "default:sand", "default:redsand", "default:snow", "default:snowblock", "default:dirt_with_snow",  "default:dirt_with_grass"},
	min_light = 10,
	interval = 30,
	chance = 10000,
	min_height = 0,
	max_height = 31000,
	day_toggle = true,
})

mobs:register_egg("mobs_animal:kitten", "Cat's egg", "mobs_kitten_egg.png", 1)

mobs:alias_mob("mobs:kitten", "mobs_animal:kitten") -- compatibility
