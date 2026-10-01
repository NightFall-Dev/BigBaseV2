#include "fonts/fonts.hpp"
#include "natives.hpp"
#include "pointers.hpp"
#include "services/gui/gui_service.hpp"
#include "services/player_database/player_database_service.hpp"
#include "services/players/player_service.hpp"
#include "views/view.hpp"

#include <player/CPlayerInfo.hpp>
#include <vehicle/CVehicleModelInfo.hpp>

#define IMGUI_DEFINE_PLACEMENT_NEW
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

namespace big
{
	bool has_scrollbar = false;

	namespace
	{
		enum class player_presence
		{
			loading,
			on_foot,
			swimming,
			interior,
			car,
			boat,
			aircraft,
			vehicle
		};

		player_presence get_player_presence(const player_ptr& plyr)
		{
			const auto player_info = plyr->get_player_info();
			const auto ped = plyr->get_ped();
			if (player_info == nullptr || ped == nullptr || player_info->m_game_state == eGameState::Invalid)
				return player_presence::loading;

			if (const auto vehicle = plyr->get_current_vehicle(); vehicle != nullptr)
			{
				if (const auto model_info = vehicle->m_model_info; model_info != nullptr)
				{
					switch (static_cast<CVehicleModelInfo*>(model_info)->m_vehicle_type)
					{
					case eVehicleType::VEHICLE_TYPE_BOAT:
					case eVehicleType::VEHICLE_TYPE_SUBMARINE:
					case eVehicleType::VEHICLE_TYPE_SUBMARINECAR:
						return player_presence::boat;
					case eVehicleType::VEHICLE_TYPE_PLANE:
					case eVehicleType::VEHICLE_TYPE_HELI:
					case eVehicleType::VEHICLE_TYPE_BLIMP:
					case eVehicleType::VEHICLE_TYPE_AUTOGYRO:
						return player_presence::aircraft;
					default:
						return player_presence::car;
					}
				}
				return player_presence::vehicle;
			}

			if ((ped->m_ped_task_flag & static_cast<uint8_t>(ePedTask::TASK_DRIVING))
			    || PLAYER::IS_REMOTE_PLAYER_IN_NON_CLONED_VEHICLE(plyr->id()))
				return player_presence::vehicle;

			const auto ped_handle = PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(plyr->id());
			if (ped_handle == 0)
				return player_presence::loading;
			if (PED::IS_PED_SWIMMING(ped_handle))
				return player_presence::swimming;
			if (INTERIOR::GET_INTERIOR_FROM_ENTITY(ped_handle) != 0)
				return player_presence::interior;
			return player_presence::on_foot;
		}

		const char* get_presence_tooltip(player_presence presence)
		{
			switch (presence)
			{
			case player_presence::loading: return "Loading / joining";
			case player_presence::on_foot: return "On foot";
			case player_presence::swimming: return "Swimming";
			case player_presence::interior: return "Inside an interior";
			case player_presence::car: return "In a car / land vehicle";
			case player_presence::boat: return "In a boat / submarine";
			case player_presence::aircraft: return "In an aircraft";
			case player_presence::vehicle: return "In a vehicle";
			}
			return "";
		}

		void draw_presence_icon(ImDrawList* draw_list, player_presence presence, ImVec2 center, float size, ImU32 color)
		{
			const float half = size * 0.5f;
			const float thickness = std::max(1.f, size * 0.1f);
			const ImVec2 min(center.x - half, center.y - half);
			const ImVec2 max(center.x + half, center.y + half);

			switch (presence)
			{
			case player_presence::loading:
				for (int i = -1; i <= 1; ++i)
					draw_list->AddCircleFilled({center.x + (i * size * 0.28f), center.y}, thickness, color);
				break;
			case player_presence::on_foot:
				draw_list->AddCircleFilled({center.x, min.y + size * 0.18f}, thickness * 1.25f, color);
				draw_list->AddLine({center.x, min.y + size * 0.34f}, {center.x, center.y + size * 0.14f}, color, thickness);
				draw_list->AddLine({center.x, center.y + size * 0.14f}, {min.x + size * 0.18f, max.y}, color, thickness);
				draw_list->AddLine({center.x, center.y + size * 0.14f}, {max.x - size * 0.18f, max.y}, color, thickness);
				draw_list->AddLine({center.x, center.y + size * 0.02f}, {min.x + size * 0.12f, center.y + size * 0.28f}, color, thickness);
				draw_list->AddLine({center.x, center.y + size * 0.02f}, {max.x - size * 0.12f, center.y + size * 0.28f}, color, thickness);
				break;
			case player_presence::swimming:
				for (int i = 0; i < 2; ++i)
				{
					const float y = center.y + (i * size * 0.25f);
					draw_list->AddLine({min.x + size * 0.05f, y}, {center.x - size * 0.12f, y - size * 0.1f}, color, thickness);
					draw_list->AddLine({center.x - size * 0.12f, y - size * 0.1f}, {center.x + size * 0.12f, y}, color, thickness);
					draw_list->AddLine({center.x + size * 0.12f, y}, {max.x - size * 0.05f, y - size * 0.1f}, color, thickness);
				}
				break;
			case player_presence::interior:
				draw_list->AddRect(min, max, color, 0.f, 0, thickness);
				draw_list->AddRect({center.x - size * 0.12f, center.y + size * 0.08f},
				    {center.x + size * 0.12f, max.y - size * 0.03f},
				    color,
				    0.f,
				    0,
				    thickness);
				break;
			case player_presence::car:
			case player_presence::vehicle:
				draw_list->AddRect({min.x + size * 0.08f, center.y - size * 0.18f},
				    {max.x - size * 0.08f, center.y + size * 0.2f},
				    color,
				    size * 0.08f,
				    0,
				    thickness);
				draw_list->AddLine({center.x - size * 0.28f, center.y + size * 0.02f},
				    {center.x - size * 0.16f, center.y - size * 0.22f},
				    color,
				    thickness);
				draw_list->AddLine({center.x + size * 0.28f, center.y + size * 0.02f},
				    {center.x + size * 0.16f, center.y - size * 0.22f},
				    color,
				    thickness);
				draw_list->AddCircleFilled({center.x - size * 0.22f, max.y - size * 0.1f}, thickness * 1.5f, color);
				draw_list->AddCircleFilled({center.x + size * 0.22f, max.y - size * 0.1f}, thickness * 1.5f, color);
				break;
			case player_presence::boat:
				draw_list->AddLine({min.x + size * 0.02f, center.y + size * 0.1f},
				    {max.x - size * 0.02f, center.y + size * 0.1f},
				    color,
				    thickness);
				draw_list->AddLine({min.x + size * 0.02f, center.y + size * 0.1f},
				    {center.x - size * 0.18f, max.y - size * 0.08f},
				    color,
				    thickness);
				draw_list->AddLine({max.x - size * 0.02f, center.y + size * 0.1f},
				    {center.x + size * 0.18f, max.y - size * 0.08f},
				    color,
				    thickness);
				draw_list->AddRect({center.x - size * 0.18f, min.y + size * 0.08f},
				    {center.x + size * 0.18f, center.y + size * 0.08f},
				    color,
				    0.f,
				    0,
				    thickness);
				break;
			case player_presence::aircraft:
				draw_list->AddLine({center.x, min.y}, {center.x, max.y}, color, thickness);
				draw_list->AddLine({min.x, center.y + size * 0.08f}, {max.x, center.y + size * 0.08f}, color, thickness);
				draw_list->AddLine({center.x - size * 0.18f, center.y + size * 0.25f},
				    {center.x + size * 0.18f, center.y + size * 0.25f},
				    color,
				    thickness);
				break;
			}
		}
	}

	static void player_button(const player_ptr& plyr)
	{
		if (plyr == nullptr || !plyr->is_valid())
			return;

		bool selected_player = plyr == g_player_service->get_selected();
		const auto presence = get_player_presence(plyr);

		// Generate social status icons.
		std::string player_icons;
		if (plyr->is_host())
			player_icons += FONT_ICON_HOST;
		if (plyr->is_friend())
			player_icons += FONT_ICON_FRIEND;

		const auto player_iconsc    = player_icons.c_str();
		const auto player_icons_end = player_iconsc + player_icons.size();
		const float status_icon_size = 14.f * g.window.gui_scale;
		const float status_spacing = player_icons.empty() ? 0.f : 4.f * g.window.gui_scale;

		// Calculate status and social icon positions.
		const auto window = ImGui::GetCurrentWindow();
		ImGui::PushFont(g.window.font_icon);
		const auto icons_size = ImGui::CalcTextSize(player_iconsc, player_icons_end);
		const float status_width = status_icon_size + status_spacing;
		const ImVec2 status_pos(window->DC.CursorPos.x + 300.0f * g.window.gui_scale - 32.0f - icons_size.x - status_width,
		    window->DC.CursorPos.y + 2.0f);
		const ImVec2 icons_pos(status_pos.x + status_width, status_pos.y);
		const ImRect icons_box(icons_pos, icons_pos + icons_size);
		ImGui::PopFont();

		if (plyr->is_admin)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.f, 0.67f, 0.f, 1.f));
		else if (plyr->is_modder)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.f, 0.1f, 0.1f, 1.f));
		else if (plyr->is_trusted)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.67f, 0.1f, 1.f));

		if (selected_player)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.29f, 0.45f, 0.69f, 1.f));

		ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, {0.0, 0.5});
		ImGui::PushID(plyr->id());

		const auto style = ImGui::GetStyle();
		// branchless conditional calculation
		const auto plyr_btn_width = (300.f * g.window.gui_scale) - (style.ItemInnerSpacing.x * 2) - (has_scrollbar * style.ScrollbarSize);
		if (ImGui::Button(plyr->get_name(), { plyr_btn_width, 0.f}))
		{
			g_player_service->set_selected(plyr);
			g_gui_service->set_selected(tabs::PLAYER);
			g.window.switched_view = true;
		}
		if (auto rockstar_id = plyr->get_rockstar_id(); ImGui::IsItemHovered() && g_player_database_service->get_player_by_rockstar_id(rockstar_id) != nullptr)
		{
			auto sorted_player = g_player_database_service->get_player_by_rockstar_id(rockstar_id);
			if (!sorted_player->infractions.empty())
			{
				ImGui::BeginTooltip();
				for (auto infraction : sorted_player->infractions)
					ImGui::BulletText("%s", sorted_player->get_infraction_description(infraction));
				ImGui::EndTooltip();
			}
		}

		ImGui::PopID();
		ImGui::PopStyleVar();

		if (selected_player)
			ImGui::PopStyleColor();

		if (plyr->is_admin || plyr->is_modder || plyr->is_trusted)
			ImGui::PopStyleColor();

		// render icons on top of the player button
		const auto status_color = ImGui::GetColorU32(ImGuiCol_Text);
		const ImVec2 status_center(status_pos.x + status_icon_size * 0.5f,
		    status_pos.y + (ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.y * 2.f) * 0.5f);
		auto* draw_list = ImGui::GetForegroundDrawList();
		draw_presence_icon(draw_list, presence, status_center, status_icon_size, status_color);
		if (ImGui::IsMouseHoveringRect({status_pos.x, status_center.y - status_icon_size * 0.5f},
		        {status_pos.x + status_icon_size, status_center.y + status_icon_size * 0.5f}))
			ImGui::SetTooltip("%s", get_presence_tooltip(presence));

		ImGui::PushFont(g.window.font_icon);
		ImGui::RenderTextWrapped(icons_box.Min, player_iconsc, player_icons_end, icons_size.x);
		ImGui::PopFont();
	}

	void view::players()
	{
		// player count does not include ourself that's why +1
		const auto player_count = g_player_service->players().size() + 1;

		if (!*g_pointers->m_gta.m_is_session_started && player_count < 2)
			return;

		const auto* navigation_window = ImGui::FindWindowByName("navigation");
		const ImVec2 window_size = navigation_window
		    ? ImVec2(navigation_window->Size.x, 0.f)
		    : ImVec2(300.f * g.window.gui_scale, 0.f);
		const ImVec2 window_pos = navigation_window
		    ? ImVec2(navigation_window->Pos.x, navigation_window->Pos.y + navigation_window->Size.y + ImGui::GetStyle().ItemSpacing.y)
		    : ImVec2(10.f, 100.f * g.window.gui_scale);

		ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {2.0f, 2.0f});

		if (ImGui::Begin("playerlist", nullptr, window_flags))
		{
			const auto style = ImGui::GetStyle();
			float window_height = (
				ImGui::CalcTextSize("A").y + style.FramePadding.y * 2.0f + style.ItemSpacing.y) // button size
				* player_count // amount of players
				+ (player_count > 1) * ((style.ItemSpacing.y * 2) + 1.f) // account for ImGui::Separator spacing
				+ (player_count == 1) * 2.f; // some arbitrary height to make it fit
			// used to account for scrollbar width
			has_scrollbar = window_height + window_pos.y > (float)*g_pointers->m_gta.m_resolution_y - 10.f;

			// basically whichever is smaller, the max available screenspace or the calculated window_height
			window_height = has_scrollbar ? (float)*g_pointers->m_gta.m_resolution_y - (window_pos.y + 40.f) : window_height;

			ImGui::PushStyleColor(ImGuiCol_FrameBg, {0.f, 0.f, 0.f, 0.f});
			ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, {0.f, 0.f, 0.f, 0.f});

			if (ImGui::BeginListBox("##players", {ImGui::GetWindowSize().x - ImGui::GetStyle().WindowPadding.x * 2, window_height}))
			{
				ImGui::SetScrollX(0);
				player_button(g_player_service->get_self());

				if (player_count > 1)
					ImGui::Separator();

				for (const auto& [_, player] : g_player_service->players())
					player_button(player);

				ImGui::EndListBox();
			}
			ImGui::PopStyleColor(2);
		}

		ImGui::PopStyleVar();
		ImGui::End();
	}
}
