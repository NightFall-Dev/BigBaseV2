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
		components::command_button<"highlighttp">({}, "VIEW_TELEPORT_SELECTED"_T);
		ImGui::SameLine();
		components::button("TP_TO_SAFE_POS"_T, [] {
			teleport::to_safe_pos();
		});
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
