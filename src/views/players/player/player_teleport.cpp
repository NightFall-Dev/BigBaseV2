#include "core/data/apartment_names.hpp"
#include "core/data/warehouse_names.hpp"
#include "util/teleport.hpp"
#include "util/toxic.hpp"
#include "views/view.hpp"

namespace big
{
	void view::player_teleport()
	{
		ImGui::SeparatorText("GUI_TAB_TELEPORT"_T.data());

		components::player_command_button<"playertp">(g_player_service->get_selected());
		ImGui::SameLine();
		components::player_command_button<"playervehtp">(g_player_service->get_selected());
		ImGui::SameLine();
		components::player_command_button<"bring">(g_player_service->get_selected());
		ImGui::SameLine();
		components::button("VIEW_PLAYER_TELEPORT_YOUR_WAYPOINT"_T, [] {
			Vector3 location;
			if (blip::get_blip_location(location, (int)BlipIcons::RADAR_WAYPOINT))
				entity::load_ground_at_3dcoord(location), teleport::teleport_player_to_coords(g_player_service->get_selected(), location);
		});

		ImGui::SameLine();

		if (g_player_service->get_selected()->get_ped())
		{
			static float new_location[3];
			auto& current_location = *reinterpret_cast<float(*)[3]>(g_player_service->get_selected()->get_ped()->get_position());

			components::small_text("VIEW_PLAYER_TELEPORT_CUSTOM_TP"_T);
			ImGui::SetNextItemWidth(400);
			ImGui::InputFloat3("##customlocation", new_location);
			components::button("GUI_TAB_TELEPORT"_T, [] {
				teleport::teleport_player_to_coords(g_player_service->get_selected(), *reinterpret_cast<rage::fvector3*>(&new_location));
			});
			ImGui::SameLine();
			if (ImGui::Button("VIEW_PLAYER_TELEPORT_GET_CURRENT"_T.data()))
			{
				std::copy(std::begin(current_location), std::end(current_location), std::begin(new_location));
			}
		}
	}
}
