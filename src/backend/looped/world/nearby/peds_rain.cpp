#include "backend/looped_command.hpp"
#include "natives.hpp"
#include "pointers.hpp"
#include "util/entity.hpp"
#include <random> // Added for smooth, non-integer coordinate distribution

namespace big
{
	class ped_rain : looped_command
	{
		using looped_command::looped_command;

		std::chrono::steady_clock::time_point last_call_time;

		virtual void on_tick() override
		{
			auto current_time = std::chrono::steady_clock::now();
			auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_call_time).count();
			
			// Only run this logic every 300ms to give peds time to actually fall and prevent script freezing
			if (elapsed_time >= 300)
			{
				last_call_time = current_time;

				auto peds = entity::get_peds(); // Use a dedicated helper vector if available, or keep get_entities(false, true)
				if (peds.empty())
					return;

				// Setup standard C++ random engine
				std::random_device rd;
				std::mt19937 gen(rd());
				std::uniform_real_distribution<float> dis_offset(-40.0f, 40.0f);
				std::uniform_real_distribution<float> dis_height(60.0f, 90.0f);

				// Shuffle and grab a max of 4 random peds per interval so it looks like consistent rain
				std::shuffle(peds.begin(), peds.end(), gen);
				size_t targets_this_tick = std::min<size_t>(peds.size(), 4);

				for (size_t i = 0; i < targets_this_tick; ++i)
				{
					auto ped = peds[i];

					// Check safety constraints first before requesting network control (saves CPU cycles)
					if (ped != self::ped && !PED::IS_PED_A_PLAYER(ped) && !ENTITY::IS_ENTITY_IN_AIR(ped) && !ENTITY::IS_ENTITY_DEAD(ped, 0))
					{
						// If they are in a vehicle, teleport the vehicle with them instead of breaking the game state
						Entity target_entity = ped;
						if (PED::IS_PED_IN_ANY_VEHICLE(ped, false))
						{
							target_entity = PED::GET_VEHICLE_PED_IS_USING(ped);
						}

						if (entity::take_control_of(target_entity, 0))
						{
							Vector3 my_location = ENTITY::GET_ENTITY_COORDS(self::ped, true);
							
							float target_x = my_location.x + dis_offset(gen);
							float target_y = my_location.y + dis_offset(gen);
							float target_z = my_location.z + dis_height(gen);

							ENTITY::SET_ENTITY_COORDS(target_entity, target_x, target_y, target_z, false, false, false, false);
							
							// Push them down hard (-25.0f) so they hurtle towards the ground naturally
							ENTITY::SET_ENTITY_VELOCITY(target_entity, 0.0f, 0.0f, -25.0f);
						}
					}
				}
			}
		}
	};

	ped_rain g_ped_rain("pedrain", "BACKEND_LOOPED_WORLD_RAIN_PEDS", "BACKEND_LOOPED_WORLD_RAIN_PEDS_DESC", g.world.nearby.ped_rain);
}