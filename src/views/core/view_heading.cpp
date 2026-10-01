#include "views/view.hpp"
#include "lua/lua_manager.hpp"
#include "natives.hpp"
#include "services/players/player_service.hpp"

#include <array>
#include <cmath>
#include <string_view>

namespace big
{
	void view::heading()
	{
		ImGui::SetNextWindowSize({300.f * g.window.gui_scale, 100.f * g.window.gui_scale});
		ImGui::SetNextWindowPos({10.f, 10.f});
		if (ImGui::Begin("menu_heading", nullptr, window_flags | ImGuiWindowFlags_NoScrollbar))
		{
			ImGui::BeginGroup();
			ImGui::Text("HEADING_WELCOME"_T.data());
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.172f, 0.380f, 0.909f, 1.f));
			ImGui::Text(g_local_player == nullptr || g_local_player->m_player_info == nullptr ?
			        "UNKNOWN_USERNAME"_T.data() :
			        g_local_player->m_player_info->m_net_player_data.m_name);
			ImGui::PopStyleColor();
			if (g_local_player != nullptr && g_local_player->m_player_info != nullptr
			    && g_local_player->m_player_info->m_game_state == eGameState::Playing)
			{
				constexpr std::array<std::string_view, 7> weekdays = {
				    "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"
				};
				const auto day = CLOCK::GET_CLOCK_DAY_OF_WEEK();
				const auto hour = CLOCK::GET_CLOCK_HOURS();
				const auto minute = CLOCK::GET_CLOCK_MINUTES();
				const auto weekday = day >= 0 && day < static_cast<int>(weekdays.size()) ? weekdays[day] : std::string_view{"Unknown"};
				const auto hour_12 = hour % 12 == 0 ? 12 : hour % 12;
				const auto meridiem = hour < 12 ? "AM" : "PM";
				ImGui::Text(std::format("{} | {}:{:02} {}", weekday, hour_12, minute, meridiem).c_str());

				if (*g_pointers->m_gta.m_is_session_started)
				{
					const auto self = g_player_service->get_self();
					if (self != nullptr && self->is_host())
					{
						ImGui::SameLine();
						ImGui::TextDisabled("HOST: YOU");
					}
					else
					{
						player_ptr host;
						for (const auto& [_, player] : g_player_service->players())
						{
							if (player != nullptr && player->is_valid() && player->is_host())
							{
								host = player;
								break;
							}
						}

						if (host != nullptr)
						{
							const auto ping = NETWORK::NETWORK_GET_AVERAGE_PING(host->id());
							ImGui::SameLine();
							if (std::isfinite(ping) && ping >= 0.f)
								ImGui::TextDisabled("HOST: %.0f ms", ping);
							else
								ImGui::TextDisabled("HOST: --");
						}
					}
				}
			}
			ImGui::EndGroup();
#ifdef YIM_DEV
			ImGui::SameLine();
			ImGui::SetCursorPos(
			    {(300.f * g.window.gui_scale) - ImGui::CalcTextSize("UNLOAD"_T.data()).x - ImGui::GetStyle().ItemSpacing.x,
			        ImGui::GetStyle().WindowPadding.y / 2 + ImGui::GetStyle().ItemSpacing.y + (ImGui::CalcTextSize("W").y / 2)});
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.69f, 0.29f, 0.29f, 1.00f));
			if (components::nav_button("UNLOAD"_T))
			{
				g_lua_manager->trigger_event<menu_event::MenuUnloaded>();
				g_running = false;
			}
			ImGui::PopStyleColor();
#endif
		}
		ImGui::End();
	}
}
