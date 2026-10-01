#include "util/mobile.hpp"
#include "views/view.hpp"

namespace big
{
	void view::mobile()
	{
		if (!*g_pointers->m_gta.m_is_session_started)
		{
			ImGui::Text("NOT_ONLINE"_T.data());
			return;
		}
		if (ImGui::BeginTabBar("##mobile_tab_bar"))
		{
			if (ImGui::BeginTabItem("MERRYWEATHER"_T.data()))
			{
				components::command_button<"ammodrop">();
				ImGui::SameLine();
				components::button("MW_HELI_PICKUP"_T, [] {
					mobile::merry_weather::request_helicopter_pickup();
				});
				ImGui::SameLine();
				components::command_button<"boatpickup">();
				ImGui::SameLine();
				components::button("MW_BACKUP_HELI"_T, [] {
					mobile::merry_weather::request_backup_helicopter();
				});
				ImGui::SameLine();
				components::button("MW_AIRSTRIKE"_T, [] {
					mobile::merry_weather::request_airstrike();
				});
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("CEO_ABILITIES"_T.data()))
			{
				components::button("CEO_BULLSHARK"_T, [] {
					mobile::ceo_abilities::request_bullshark_testosterone();
				});
				ImGui::SameLine();
				components::command_button<"ballisticarmor">();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("VIEW_SELF_MOBILE_SERVICES"_T.data()))
			{
				if (ImGui::BeginTable("##mobile_services", 3))
				{
					components::command_button<"avenger">();
					ImGui::TableNextColumn();
					components::command_button<"kosatka">();
					ImGui::TableNextColumn();
					components::command_button<"moc">();
					ImGui::TableNextRow();

					components::command_button<"terrorbyte">();
					ImGui::TableNextColumn();
					components::command_button<"acidlab">();
					ImGui::TableNextColumn();
					components::command_button<"acidbike">();
					ImGui::TableNextRow();

					components::command_button<"dinghy">();
					ImGui::TableNextColumn();
					components::command_button<"transporter">();
					ImGui::TableNextColumn();
					components::command_button<"rcbandito">();
					ImGui::TableNextRow();

					components::command_button<"rctank">();
					ImGui::TableNextColumn();
					components::command_button<"supervolito">();
					ImGui::TableNextColumn();
					components::command_button<"cphelibk">();

					ImGui::EndTable();
				}
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("DEBUG_TAB_MISC"_T.data()))
			{
				components::command_button<"taxi">();
				ImGui::SameLine();
				components::command_button<"gunvan">();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("MORS_MUTUAL"_T.data()))
			{
				components::button("MORS_FIX_ALL"_T, [] {
					int amount_fixed = mobile::mors_mutual::fix_all();
					auto v_fixed     = amount_fixed == 1 ? "VEHICLE_FIX_HAS"_T.data() : "VEHICLE_FIX_HAVE"_T.data();

					g_notification_service.push_success("MOBILE"_T.data(),
					    std::vformat("VEHICLE_FIX_AMOUNT"_T, std::make_format_args(amount_fixed, v_fixed)));
				});
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}
}
