#include "views/view.hpp"
#include "services/players/player_service.hpp"
#include "util/scripts.hpp"

namespace big
{
	void view::player_misc()
	{ 
		ImGui::SeparatorText("DEBUG_TAB_MISC"_T.data());

		ImGui::BeginGroup();
		components::player_command_button<"joinceo">(g_player_service->get_selected());
		components::player_command_button<"enterint">(g_player_service->get_selected());
		components::player_command_button<"clearwanted">(g_player_service->get_selected());
		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::BeginGroup();
		ImGui::Checkbox("OFF_THE_RADAR"_T.data(), &g_player_service->get_selected()->off_radar);

		ImGui::EndGroup();
	}
}