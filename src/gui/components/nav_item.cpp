#include "components.hpp"
#include "services/gui/gui_service.hpp"
#include "services/translation_service/translation_service.hpp"

namespace big
{
	void components::nav_item(std::pair<tabs, navigation_struct>& navItem, int nested)
	{
		const bool current_tab =
			!g_gui_service->get_selected_tab().empty() &&
			g_gui_service->get_selected_tab().size() > nested &&
			navItem.first == g_gui_service->get_selected_tab().at(nested);


		if (current_tab)
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.29f, 0.45f, 0.69f, 1.f));

		const char* key = nullptr;
		if (key = g_translation_service.get_translation(navItem.second.name).data(); !key)
			key = navItem.second.name;
		const bool has_sub_nav = !navItem.second.sub_nav.empty();
		const float arrow_offset = 4.f * g.window.gui_scale;
		if (components::nav_button(key, has_sub_nav ? 18.f * g.window.gui_scale : 0.f))
			g_gui_service->set_selected(navItem.first);

		if (has_sub_nav)
		{
			const auto rect_min = ImGui::GetItemRectMin();
			const auto rect_max = ImGui::GetItemRectMax();
			const auto center = ImVec2(rect_min.x + arrow_offset, (rect_min.y + rect_max.y) * 0.5f);
			const auto color = ImGui::GetColorU32(ImGuiCol_Text);
			auto* draw_list = ImGui::GetForegroundDrawList();
			if (current_tab)
				draw_list->AddTriangleFilled({center.x - (4.f * g.window.gui_scale), center.y - (2.f * g.window.gui_scale)},
				    {center.x + (4.f * g.window.gui_scale), center.y - (2.f * g.window.gui_scale)},
				    {center.x, center.y + (3.f * g.window.gui_scale)},
				    color);
			else
				draw_list->AddTriangleFilled({center.x - (2.f * g.window.gui_scale), center.y - (4.f * g.window.gui_scale)},
				    {center.x - (2.f * g.window.gui_scale), center.y + (4.f * g.window.gui_scale)},
				    {center.x + (3.f * g.window.gui_scale), center.y},
				    color);
		}

		if (current_tab)
			ImGui::PopStyleColor();

		if (current_tab && !navItem.second.sub_nav.empty())
		{
			ImDrawList* draw_list = ImGui::GetForegroundDrawList();

			for (std::pair<tabs, navigation_struct> item : navItem.second.sub_nav)
			{
				draw_list->AddRectFilled({10.f, ImGui::GetCursorPosY() + (100.f * g.window.gui_scale)},
				    {(10.f + (300.f * g.window.gui_scale)),
				        (ImGui::GetCursorPosY() + (100.f * (g.window.gui_scale)) + ImGui::CalcTextSize("A").y
				            + (ImGui::GetStyle().ItemInnerSpacing.y / g.window.gui_scale) * 2)},
				    ImGui::ColorConvertFloat4ToU32({1.f, 1.f, 1.f, .15f + (.075f * nested)}));
				nav_item(item, nested + 1);
			}
		}

		g_gui_service->increment_nav_size();
	}
}
