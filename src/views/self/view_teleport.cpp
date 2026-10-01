#include "util/teleport.hpp"
#include "views/view.hpp"

namespace big
{
	void view::teleport()
	{
		ImGui::SeparatorText("BLIPS"_T.data());
		ImGui::Spacing();

		components::command_button<"waypointtp">({}, "VIEW_PLAYER_TELEPORT_WAYPOINT"_T);
		ImGui::SameLine();
		components::command_button<"objectivetp">({}, "VIEW_TELEPORT_OBJECTIVE"_T);
		ImGui::SameLine();
		components::button("TP_TO_SAFE_POS"_T, [] {
			teleport::to_safe_pos();
		});
		ImGui::SameLine();

		static Blip selected_blip_id = -1;
		auto* blip_list = g_pointers->m_gta.m_blip_list;
		std::string selected_blip_label = "Select active blip";
		if (auto* selected_blip = blip::get_blip_from_blip_id(selected_blip_id))
		{
			selected_blip_label = std::format("Blip {} | Icon {} | ({:.2f}, {:.2f}, {:.2f})",
			    selected_blip->m_blip_array_index,
			    selected_blip->m_icon,
			    selected_blip->m_position.x,
			    selected_blip->m_position.y,
			    selected_blip->m_position.z);
		}

		ImGui::SetNextItemWidth(450);
		if (ImGui::BeginCombo("##active_blip_select", selected_blip_label.c_str()))
		{
			if (blip_list)
			{
				for (int i = 0; i < 1500; i++)
				{
					auto* active_blip = blip_list->m_Blips[i].m_pBlip;
					if (!active_blip)
						continue;

					const auto blip_id = static_cast<Blip>(active_blip->m_blip_array_index);
					const auto label = std::format("Blip {} | Icon {} | ({:.2f}, {:.2f}, {:.2f})##{}",
					    blip_id,
					    active_blip->m_icon,
					    active_blip->m_position.x,
					    active_blip->m_position.y,
					    active_blip->m_position.z,
					    blip_id);
					if (ImGui::Selectable(label.c_str(), selected_blip_id == blip_id))
						selected_blip_id = blip_id;
				}
			}
			ImGui::EndCombo();
		}

		ImGui::SameLine();
		components::command_button<"highlighttp">(
		    selected_blip_id == -1 ? std::vector<uint64_t>{} : std::vector<uint64_t>{static_cast<uint64_t>(selected_blip_id)},
		    "VIEW_TELEPORT_SELECTED"_T);
		components::command_checkbox<"autotptowp">();

		ImGui::SeparatorText("VIEW_TELEPORT_MOVEMENT"_T.data());

		ImGui::Spacing();

		components::small_text("VIEW_TELEPORT_CURRENT_COORDINATES"_T);
		float coords[3] = {self::pos.x, self::pos.y, self::pos.z};
		static float new_location[3];
		static float increment = 1;

		ImGui::SetNextItemWidth(400);
		ImGui::InputFloat3("##currentcoordinates", coords, "%f", ImGuiInputTextFlags_ReadOnly);
		ImGui::SameLine();
		components::button("VIEW_TELEPORT_COPY_TO_CUSTOM"_T, [coords] {
			std::copy(std::begin(coords), std::end(coords), std::begin(new_location));
		});

		components::small_text("GUI_TAB_CUSTOM_TELEPORT"_T);
		ImGui::SetNextItemWidth(400);
		ImGui::InputFloat3("##Customlocation", new_location);
		ImGui::SameLine();
		components::button("GUI_TAB_TELEPORT"_T, [] {
			teleport::to_coords({new_location[0], new_location[1], new_location[2]});
		});

		ImGui::Spacing();
		components::small_text("VIEW_TELEPORT_SPECIFIC_MOVEMENT"_T);
		ImGui::Spacing();

		ImGui::SetNextItemWidth(200);
		ImGui::InputFloat("VIEW_SELF_CUSTOM_TELEPORT_DISTANCE"_T.data(), &increment);

		ImGui::BeginGroup();
		components::button("VIEW_TELEPORT_FORWARD"_T, [] {
			teleport::to_coords(ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(self::ped, 0, increment, 0));
		});
		components::button("VIEW_TELEPORT_BACKWARD"_T, [] {
			teleport::to_coords(ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(self::ped, 0, -increment, 0));
		});
		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::BeginGroup();
		components::button("LEFT"_T, [] {
			teleport::to_coords(ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(self::ped, -increment, 0, 0));
		});
		components::button("RIGHT"_T, [] {
			teleport::to_coords(ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(self::ped, increment, 0, 0));
		});
		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::BeginGroup();
		components::button("VIEW_TELEPORT_UP"_T, [] {
			teleport::to_coords(ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(self::ped, 0, 0, increment));
		});
		components::button("VIEW_TELEPORT_DOWN"_T, [] {
			teleport::to_coords(ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(self::ped, 0, 0, -increment));
		});
		ImGui::EndGroup();

		ImGui::SeparatorText("VEHICLES"_T.data());
		ImGui::Spacing();

		components::command_button<"lastvehtp">();
		ImGui::SameLine();
		components::command_button<"bringpv">();
		ImGui::SameLine();
		components::command_button<"pvtp">();
	}
}
