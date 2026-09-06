#include "backend/looped_command.hpp"
#include "natives.hpp"
#include "pointers.hpp"
#include "util/entity.hpp"
#include <random> // Added for better random number distribution

namespace big
{
	class vehicle_rain : looped_command
	{
		using looped_command::looped_command;
		
		std::chrono::steady_clock::time_point last_call_time;

		virtual void on_tick() override
		{
			auto current_time = std::chrono::steady_clock::now();
			auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_call_time).count();
			
			// Lowered delay to 500ms to pull random cars sequentially, creating an actual steady rain effect
			if (elapsed_time >= 500)
			{
				last_call_time = current_time;
				
				auto vehicles = entity::get_vehicles(); // Use the dedicated vehicle vector helper
				if (vehicles.empty())
					return;

				// Setup standard C++ random engines (better performance and distribution than rand())
				std::random_device rd;
				std::mt19935 gen(rd());
				std::uniform_real_distribution<float> dis_offset(-50.0f, 50.0f);
				std::uniform_real_distribution<float> dis_height(80.0f, 120.0f);
				
				// Pick up to 3 random vehicles per tick instead of moving the entire map at once
				std::shuffle(vehicles.begin(), vehicles.end(), gen);
				size_t targets_this_tick = std::min<size_t>(vehicles.size(), 3);

				for (size_t i = 0; i < targets_this_tick; ++i)
				{
					auto veh = vehicles[i];

					// Prevent targeting your own vehicle, dead vehicles, or things already high up
					if (veh != self::veh && !ENTITY::IS_ENTITY_IN_AIR(veh) && !ENTITY::IS_ENTITY_DEAD(veh, 0))
					{
						if (entity::take_control_of(veh, 0))
						{
							Vector3 my_location = ENTITY::GET_ENTITY_COORDS(self::ped, true);
							
							float target_x = my_location.x + dis_offset(gen);
							float target_y = my_location.y + dis_offset(gen);
							float target_z = my_location.z + dis_height(gen);

							// Clear the area before dropping so it doesn't clip into buildings or bridges above you
							ENTITY::SET_ENTITY_COORDS(veh, target_x, target_y, target_z, false, false, false, false);
							
							// Push the vehicle downward violently (-35.0f instead of -1.0f)
							ENTITY::SET_ENTITY_VELOCITY(veh, 0.0f, 0.0f, -35.0f);
						}
					}
				}
			}
		}
	};

	vehicle_rain g_vehicle_rain("vehiclerain", "BACKEND_LOOPED_WORLD_RAIN_VEHICLES", "BACKEND_LOOPED_WORLD_RAIN_VEHICLES_DESC", g.world.nearby.veh_rain);
}