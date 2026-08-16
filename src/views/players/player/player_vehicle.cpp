#include "views/view.hpp"

namespace big
{
	void view::player_vehicle()
	{
		ImGui::SeparatorText("VEHICLE"_T.data());

		ImGui::BeginGroup();
		components::player_command_button<"opendoors">(g_player_service->get_selected(), {});
		components::player_command_button<"closedoors">(g_player_service->get_selected(), {});
		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::BeginGroup();
		components::player_command_button<"upgradeveh">(g_player_service->get_selected(), {});
		components::player_command_button<"downgradeveh">(g_player_service->get_selected(), {});
		ImGui::EndGroup();
	}
}