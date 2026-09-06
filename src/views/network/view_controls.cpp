#include "fiber_pool.hpp"
#include "gta_util.hpp"
#include "util/troll.hpp"
#include "util/toxic.hpp"
#include "views/view.hpp"

#include <network/Network.hpp>
#include <script/globals/GPBD_FM_3.hpp>
#include <script/globals/GlobalPlayerBD.hpp>


namespace big
{
	struct SessionType
	{
		eSessionType id;
		const char* name;
	};

	void render_player_options()
	{
		ImGui::BeginGroup();
		ImGui::SameLine();
		ImGui::EndGroup();
	}

	void render_host_options()
	{
		if (!*g_pointers->m_gta.m_is_session_started)
		{
			ImGui::Text("NOT_ONLINE"_T.data());
			return;
		}

		ImGui::BeginGroup();

		ImGui::SameLine();

		ImGui::Checkbox("FAST_JOIN"_T.data(), &g.session.fast_join);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("FAST_JOIN_DESC"_T.data());
		components::command_button<"emptysession">();

		ImGui::EndGroup();
	}

	void render_session_globals()
	{
		ImGui::BeginGroup();

		ImGui::SameLine();
		ImGui::EndGroup();
	}

	void view::network_controls()
	{
		render_player_options();
		render_host_options();
		render_session_globals();
	}
}
