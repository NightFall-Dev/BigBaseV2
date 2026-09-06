#include "backend/looped_command.hpp"
#include "gta_util.hpp"
#include "natives.hpp"
#include "pointers.hpp"
#include "util/entity.hpp"
#include "util/mobile.hpp"

namespace big
{
	class blackhole : looped_command
	{
		using looped_command::looped_command;

		std::vector<Entity> entity_list;
		std::chrono::steady_clock::time_point last_call_time;

		virtual void on_tick() override
		{
			auto current_time = std::chrono::steady_clock::now();
			auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_call_time).count();

			// 1. Gather entities and request network control once every second to prevent performance lag
			if (elapsed_time >= 1000) 
			{
				entity_list = entity::get_entities(g.world.blackhole.include_vehicles, g.world.blackhole.include_peds);
				last_call_time = current_time;

				// Pre-request control over a capped amount of entities so we own them during the physics frames
				for (size_t i = 0; i (g.world.blackhole.color[0] * 255),
					static_cast<int>(g.world.blackhole.color[1] * 255),
					static_cast<int>(g.world.blackhole.color[2] * 255),
					g.world.blackhole.alpha,
					0, 0, 2, 0, 0, 0, 0);
			}
		}
	};

	blackhole g_blackhole("blackhole", "GUI_TAB_BLACKHOLE", "BACKEND_LOOPED_WORLD_BLACKHOLE_DESC", g.world.blackhole.enable);
	bool_command g_blackhole_peds("blackholeincpeds", "PEDS", "BACKEND_LOOPED_WORLD_BLACKHOLE_PEDS_DESC", g.world.blackhole.include_peds);
	bool_command g_blackhole_vehicles("blackholeincvehs", "VEHICLES", "BACKEND_LOOPED_WORLD_BLACKHOLE_VEHS_DESC", g.world.blackhole.include_vehicles);
}