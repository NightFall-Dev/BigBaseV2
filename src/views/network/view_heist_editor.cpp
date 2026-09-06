#include "gta_util.hpp"
#include "fiber_pool.hpp"
#include "natives.hpp"
#include "script_global.hpp"
#include "script_local.hpp"
#include "services/gui/gui_service.hpp"
#include "views/view.hpp"
#include "util/input_method_editor.hpp"
#include "util/ped.hpp"
#include <cstring>

namespace big
{
	namespace
	{
		rage::joaat_t mp_stat(const std::string_view name)
		{
			return rage::joaat(std::format("MP{}_{}", self::char_index, name));
		}

		void set_stat_int(const std::string_view name, int value)
		{
			STATS::STAT_SET_INT(mp_stat(name), value, TRUE);
		}

		int get_stat_int(const std::string_view name)
		{
			int value = 0;
			STATS::STAT_GET_INT(mp_stat(name), &value, -1);
			return value;
		}

		int get_global_stat_int(const std::string_view name)
		{
			int value = 0;
			STATS::STAT_GET_INT(rage::joaat(name), &value, -1);
			return value;
		}

		std::string current_timestamp()
		{
			const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			std::tm time{};
			gmtime_s(&time, &now);

			std::ostringstream timestamp;
			timestamp << std::put_time(&time, "%Y-%m-%d %H:%M:%S UTC");
			return timestamp.str();
		}

		std::int64_t current_epoch_seconds()
		{
			return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		}

		std::string live_cooldown_status(const std::string_view heist)
		{
			const auto now = current_epoch_seconds();
			std::int64_t cooldown = 0;
			std::string_view mode = "Normal";

			if (heist == "diamond_casino")
			{
				cooldown = get_global_stat_int("MPPLY_H3_COOLDOWN");
				mode = get_stat_int("H3OPT_APPROACH") != 0 && get_stat_int("H3OPT_APPROACH") == get_stat_int("H3_HARD_APPROACH") ? "Hard" : "Normal";
			}
			else
			{
				const auto normal_cooldown = static_cast<std::int64_t>(get_stat_int("H4_COOLDOWN"));
				const auto hard_cooldown = static_cast<std::int64_t>(get_stat_int("H4_COOLDOWN_HARD"));
				if (hard_cooldown > now)
				{
					cooldown = hard_cooldown;
					mode = "Hard";
				}
				else
					cooldown = normal_cooldown;
			}

			if (cooldown <= now)
				return std::format("{}: Ready", heist == "diamond_casino" ? "Diamond Casino" : "Cayo Perico");

			const auto remaining = cooldown - now;
			return std::format("{}: {} cooldown, {}m {}s remaining", heist == "diamond_casino" ? "Diamond Casino" : "Cayo Perico", mode, remaining / 60, remaining % 60);
		}

		void set_stats(std::initializer_list<std::pair<std::string_view, int>> stats)
		{
			for (const auto& [name, value] : stats)
				set_stat_int(name, value);
		}

		static constexpr std::array<std::string_view, 19> casino_database_stats = {
			"H3OPT_APPROACH",
			"H3_LAST_APPROACH",
			"H3_HARD_APPROACH",
			"H3OPT_TARGET",
			"H3OPT_POI",
			"H3OPT_ACCESSPOINTS",
			"CAS_HEIST_FLOW",
			"H3OPT_COMPLETEDPOSIX",
			"H3OPT_BITSET1",
			"H3OPT_KEYLEVELS",
			"H3OPT_DISRUPTSHIP",
			"H3OPT_BODYARMORLVL",
			"H3OPT_CREWWEAP",
			"H3OPT_CREWDRIVER",
			"H3OPT_CREWHACKER",
			"H3OPT_VEHS",
			"H3OPT_WEAPS",
			"H3OPT_BITSET0",
			"H3OPT_MASKS",
		};

		static constexpr std::array<std::string_view, 39> cayo_database_stats = {
			"H4CNF_BS_GEN",
			"H4CNF_WEAPONS",
			"H4CNF_WEP_DISRP",
			"H4CNF_ARM_DISRP",
			"H4CNF_HEL_DISRP",
			"H4CNF_TARGET",
			"H4CNF_BOLTCUT",
			"H4CNF_UNIFORM",
			"H4CNF_GRAPPEL",
			"H4CNF_TROJAN",
			"H4LOOT_CASH_I",
			"H4LOOT_CASH_I_SCOPED",
			"H4LOOT_CASH_C",
			"H4LOOT_CASH_C_SCOPED",
			"H4LOOT_COKE_I",
			"H4LOOT_COKE_I_SCOPED",
			"H4LOOT_COKE_C",
			"H4LOOT_COKE_C_SCOPED",
			"H4LOOT_GOLD_I",
			"H4LOOT_GOLD_I_SCOPED",
			"H4LOOT_GOLD_C",
			"H4LOOT_GOLD_C_SCOPED",
			"H4LOOT_WEED_I",
			"H4LOOT_WEED_I_SCOPED",
			"H4LOOT_WEED_C",
			"H4LOOT_WEED_C_SCOPED",
			"H4LOOT_PAINT",
			"H4LOOT_PAINT_SCOPED",
			"H4LOOT_PAINT_V",
			"H4LOOT_WEED_V",
			"H4LOOT_GOLD_V",
			"H4LOOT_COKE_V",
			"H4LOOT_CASH_V",
			"H4_PROGRESS",
			"H4_MISSIONS",
			"H4_PLAYTHROUGH_STATUS",
			"H4CNF_BS_ABIL",
			"H4CNF_BS_ENTR",
			"H4CNF_APPROACH",
		};

		template<std::size_t N>
		std::size_t save_heist_database_entry(const std::string_view heist, const std::array<std::string_view, N>& stats, std::string_view notes = {})
		{
			auto file = g_file_manager.get_project_file("heist_database.json");
			nlohmann::json database = nlohmann::json::object();
			if (file.exists())
			{
				try
				{
					std::ifstream input(file.get_path());
					input >> database;
					const auto character = std::format("MP{}", self::char_index);
				}
				catch (const std::exception& exception)
				{
					LOG(WARNING) << "Failed to read heist database: " << exception.what();
				}
			}

			const auto character = std::format("MP{}", self::char_index);
			if (database[character].contains(heist) && database[character][heist].is_object())
			{
				auto previous = database[character][heist];
				database[character][heist] = nlohmann::json::array();
				database[character][heist].push_back(std::move(previous));
			}
			if (!database[character].contains(heist) || !database[character][heist].is_array())
				database[character][heist] = nlohmann::json::array();

			nlohmann::json entry = nlohmann::json::object();
			for (const auto stat : stats)
				entry[stat] = get_stat_int(stat);

			entry["notes"] = std::string(notes);
			entry["saved_at"] = current_timestamp();
			if (heist == "diamond_casino")
			{
				const auto approach = get_stat_int("H3OPT_APPROACH");
				const auto hard_approach = get_stat_int("H3_HARD_APPROACH");
				entry["cooldown_mode"] = approach != 0 && approach == hard_approach ? "Hard" : "Normal";
				entry["cooldown_value"] = get_global_stat_int("MPPLY_H3_COOLDOWN");
			}
			else
			{
				const auto hard_mode = get_stat_int("H4_PROGRESS") == 131055;
				entry["cooldown_mode"] = hard_mode ? "Hard" : "Normal";
				entry["cooldown_value"] = get_stat_int("H4_COOLDOWN");
				entry["hard_cooldown_value"] = get_stat_int("H4_COOLDOWN_HARD");
			}
			database[character][heist].push_back(std::move(entry));
			const auto entry_index = database[character][heist].size() - 1;

			std::ofstream output(file.get_path(), std::ios::out | std::ios::trunc);
			output << database.dump(4);
			return entry_index;
		}

		template<std::size_t N>
		void delete_heist_database_entry(const std::string_view heist, const std::array<std::string_view, N>& stats, std::size_t entry_index)
		{
			auto file = g_file_manager.get_project_file("heist_database.json");
			if (!file.exists())
				return;

			try
			{
				std::ifstream input(file.get_path());
				nlohmann::json database;
				input >> database;

				const auto character = std::format("MP{}", self::char_index);
				if (!database.contains(character) || !database[character].contains(heist))
					return;
				auto& entries = database[character][heist];
				if (entries.is_object())
				{
					if (entry_index != 0)
						return;
					database[character][heist] = nlohmann::json::array();
				}
				else if (!entries.is_array() || entry_index >= entries.size())
					return;
				else
				{
					entries.erase(entry_index);
				}

				std::ofstream output(file.get_path(), std::ios::out | std::ios::trunc);
				output << database.dump(4);
			}
			catch (const std::exception& exception)
			{
				LOG(WARNING) << "Failed to delete heist database entry: " << exception.what();
			}
		}

		template<std::size_t N>
		void load_heist_database_entry(const std::string_view heist, const std::array<std::string_view, N>& stats, std::size_t entry_index)
		{
			auto file = g_file_manager.get_project_file("heist_database.json");
			if (!file.exists())
				return;

			try
			{
				std::ifstream input(file.get_path());
				nlohmann::json database;
				input >> database;

				const auto character = std::format("MP{}", self::char_index);
				if (!database.contains(character) || !database[character].contains(heist))
					return;
				auto& entries = database[character][heist];
				if (entries.is_object())
				{
					if (entry_index != 0)
						return;
				}
				else if (!entries.is_array() || entry_index >= entries.size())
					return;
				auto& entry = entries.is_array() ? entries[entry_index] : entries;

				for (const auto stat : stats)
				{
					if (entry.contains(stat) && entry[stat].is_number_integer())
						set_stat_int(stat, entry[stat].get<int>());
				}
			}
			catch (const std::exception& exception)
			{
				LOG(WARNING) << "Failed to load heist database: " << exception.what();
			}
		}

		template<std::size_t N>
		void draw_heist_database_entries(const std::string_view heist, const std::array<std::string_view, N>& stats, std::size_t& selected_entry)
		{
			auto file = g_file_manager.get_project_file("heist_database.json");
			if (!file.exists())
				return;

			try
			{
				std::ifstream input(file.get_path());
				nlohmann::json database;
				input >> database;
				const auto character = std::format("MP{}", self::char_index);
				if (!database.contains(character) || !database[character].contains(heist))
					return;

				auto entries = database[character][heist];
				if (entries.is_object())
				{
					nlohmann::json migrated = nlohmann::json::array();
					migrated.push_back(std::move(entries));
					entries = std::move(migrated);
				}
				if (!entries.is_array() || entries.empty())
					return;

				selected_entry = std::min(selected_entry, entries.size() - 1);
				const auto preview = entries[selected_entry].value("saved_at", "Unknown");
				if (ImGui::BeginCombo("Saved Entry", preview.c_str()))
				{
					for (std::size_t index = 0; index < entries.size(); ++index)
					{
						const auto label = std::format("{}: {}", index + 1, entries[index].value("saved_at", "Unknown"));
						if (ImGui::Selectable(label.c_str(), selected_entry == index))
							selected_entry = index;
					}
					ImGui::EndCombo();
				}

				const auto& entry = entries[selected_entry];
				if (ImGui::BeginTable("##heist_database_values", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, {0.0F, 360.0F}))
				{
					ImGui::TableSetupColumn("Stat");
					ImGui::TableSetupColumn("Value");
					ImGui::TableHeadersRow();
					const auto draw_row = [](const std::string_view name, const std::string& value) {
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::TextUnformatted(name.data());
						ImGui::TableSetColumnIndex(1);
						ImGui::TextUnformatted(value.c_str());
					};
					draw_row("notes", entry.value("notes", ""));
					draw_row("saved_at", entry.value("saved_at", "Unknown"));
					draw_row("cooldown_mode", entry.value("cooldown_mode", "Unknown"));
					draw_row("cooldown_value", std::to_string(entry.value("cooldown_value", 0)));
					if (entry.contains("hard_cooldown_value"))
						draw_row("hard_cooldown_value", std::to_string(entry.value("hard_cooldown_value", 0)));
					for (const auto stat : stats)
					{
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::TextUnformatted(stat.data());
						ImGui::TableSetColumnIndex(1);
						ImGui::TextUnformatted(entry.contains(stat) ? std::to_string(entry[stat].get<int>()).c_str() : "-");
					}
					ImGui::EndTable();
				}
			}
			catch (const std::exception& exception)
			{
				LOG(WARNING) << "Failed to render heist database: " << exception.what();
			}
		}

		void set_global_int(std::size_t index, int value)
		{
			*script_global(index).as<int*>() = value;
		}

		void set_local_int(const rage::joaat_t script, std::size_t index, int value)
		{
			if (auto thread = gta_util::find_script_thread(script))
				*script_local(thread->m_stack, index).as<int*>() = value;
		}

		void set_local_float(const rage::joaat_t script, std::size_t index, float value)
		{
			if (auto thread = gta_util::find_script_thread(script))
				*script_local(thread->m_stack, index).as<float*>() = value;
		}

		bool toggle_switch(const char* label, bool* value)
		{
			ImGui::PushID(label);
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted(label);
			ImGui::SameLine();

			const auto position = ImGui::GetCursorScreenPos();
			const ImVec2 size(36.0F, 20.0F);
			const auto changed = ImGui::InvisibleButton("##toggle", size);
			if (changed)
				*value = !*value;

			const auto color = *value ? ImGui::GetColorU32(ImGuiCol_ButtonActive) : ImGui::GetColorU32(ImGuiCol_FrameBg);
			auto* draw_list = ImGui::GetWindowDrawList();
			draw_list->AddRectFilled(position, position + size, color, size.y * 0.5F);
			draw_list->AddCircleFilled({position.x + (*value ? size.x - 10.0F : 10.0F), position.y + size.y * 0.5F}, 7.0F, ImGui::GetColorU32(ImGuiCol_SliderGrab));

			ImGui::PopID();
			return changed;
		}

		void draw_cuts(std::array<int, 4>& cuts, std::size_t first_global, const char* id)
		{
			components::sub_title("Heist Cuts");
			for (int i = 0; i < cuts.size(); ++i)
			{
				ImGui::PushID(std::format("{}{}", id, i).c_str());
				ImGui::InputInt(std::format("Player {}", i + 1).c_str(), &cuts[i]);
				ImGui::PopID();
			}

			if (ImGui::Button("Apply Cuts"))
				for (int i = 0; i < cuts.size(); ++i)
					set_global_int(first_global + i, cuts[i]);
		}

		void draw_apartment_heist()
		{
			components::sub_title("Apartment Heist");
			static std::array<int, 4> cuts{};
			draw_cuts(cuts, 1935929 + 1 + 1, "apartment_cut");

			if (ImGui::Button("Complete Preps"))
				set_stat_int("HEIST_PLANNING_STAGE", -1);

			static int lives = 3;
			ImGui::InputInt("Team Lives", &lives);
			if (ImGui::Button("Set Lives"))
				set_local_int("fm_mission_controller"_J, 26234 + 1325 + 1, std::max(lives, 1));

			ImGui::SeparatorText("Extras");
			if (ImGui::Button("Bypass Fleeca Hack"))
				set_local_int("fm_mission_controller"_J, 11837 + 24, 7);
			ImGui::SameLine();
			if (ImGui::Button("Bypass Fleeca Drill"))
				set_local_int("fm_mission_controller"_J, 10125 + 11, 100);
			ImGui::SameLine();
			if (ImGui::Button("Bypass Pacific Hack"))
				set_local_int("fm_mission_controller"_J, 9831, 9);
			ImGui::SameLine();
			if (ImGui::Button("Skip Checkpoint"))
			{
				const auto thread = gta_util::find_script_thread("fm_mission_controller"_J);
				if (thread)
					*script_local(thread->m_stack, 19808 + 2).as<int*>() |= 1 << 17;
			}

			ImGui::SameLine();
			components::command_button<"skipcutscene">();
			ImGui::SameLine();
			if (ImGui::Button("Force Ready"))
				for (int i = 0; i < 4; ++i)
					set_global_int(2658291 + 1 + (i * 468) + 270, 6);
		}

		void draw_doomsday_heist()
		{
			components::sub_title("Doomsday Heist");
			if (ImGui::Button("Data Breaches"))
			{
				set_stat_int("GANGOPS_FLOW_MISSION_PROG", 503);
				set_stat_int("GANGOPS_HEIST_STATUS", -229383);
				set_stat_int("GANGOPS_FLOW_NOTIFICATIONS", 1557);
			}
			ImGui::SameLine();
			if (ImGui::Button("Bogdan Problem"))
			{
				set_stat_int("GANGOPS_FLOW_MISSION_PROG", 240);
				set_stat_int("GANGOPS_HEIST_STATUS", -229378);
				set_stat_int("GANGOPS_FLOW_NOTIFICATIONS", 1557);
			}
			ImGui::SameLine();
			if (ImGui::Button("Doomsday Scenario"))
			{
				set_stat_int("GANGOPS_FLOW_MISSION_PROG", 16368);
				set_stat_int("GANGOPS_HEIST_STATUS", -229380);
				set_stat_int("GANGOPS_FLOW_NOTIFICATIONS", 1557);
			}

			static std::array<int, 4> cuts{};
			draw_cuts(cuts, 1968511 + 812 + 50 + 1, "doomsday_cut");

			static int lives = 3;
			ImGui::InputInt("Team Lives", &lives);
			if (ImGui::Button("Set Lives"))
				set_local_int("fm_mission_controller"_J, 26234 + 1325 + 1, std::max(lives, 1));

			if (ImGui::Button("Complete Preps"))
				set_stat_int("GANGOPS_FM_MISSION_PROG", -1);
			ImGui::SameLine();
			if (ImGui::Button("Bypass Act III Hack"))
				set_local_int("fm_mission_controller"_J, 1312 + 135, 3);
		}

		void draw_casino_heist()
		{
			components::sub_title("Diamond Casino Heist");
			ImGui::TextUnformatted(live_cooldown_status("diamond_casino").c_str());
			static const char* approaches[] = {"Unselected", "Silent and Sneaky", "The Big Con", "Aggressive"};
			static const char* targets[] = {"Money", "Gold", "Art", "Diamonds"};
			static const char* gunmen[] = {"Unselected", "Karl Abolaji", "Gustavo Mota", "Charlie Reed", "Chester McCoy", "Patrick McReary"};
			static const char* drivers[] = {"Unselected", "Karim Denz", "Taliana Martinez", "Eddie Toh", "Zach Nelson", "Chester McCoy"};
			static const char* hackers[] = {"Unselected", "Rickie Lukens", "Christian Feltz", "Yohan Blair", "Avi Schwartzman", "Paige Harris"};
			static const char* masks[] = {"Unselected", "Geometric Set", "Hunter Set", "Oni Half Mask Set", "Emoji Set", "Ornate Skull Set", "Lucky Fruit Set", "Gurilla Set", "Clown Set", "Animal Set", "Riot Set", "Oni Set", "Hockey Set"};
			static const char* gun_loadouts[5][4][2] = {
				{{"Unselected", "Unselected"}, {"Micro SMG Loadout", "Machine Pistol Loadout"}, {"Micro SMG Loadout", "Shotgun Loadout"}, {"Shotgun Loadout", "Revolver Loadout"}},
				{{"Unselected", "Unselected"}, {"Rifle Loadout", "Shotgun Loadout"}, {"Rifle Loadout", "Shotgun Loadout"}, {"Rifle Loadout", "Shotgun Loadout"}},
				{{"Unselected", "Unselected"}, {"SMG Loadout", "Shotgun Loadout"}, {"Machine Pistol Loadout", "Shotgun Loadout"}, {"SMG Loadout", "Shotgun Loadout"}},
				{{"Unselected", "Unselected"}, {"MK II Shotgun Loadout", "MK II Rifle Loadout"}, {"MK II SMG Loadout", "MK II Rifle Loadout"}, {"MK II Shotgun Loadout", "MK II Rifle Loadout"}},
				{{"Unselected", "Unselected"}, {"Combat PDW Loadout", "Rifle Loadout"}, {"Shotgun Loadout", "Rifle Loadout"}, {"Shotgun Loadout", "Combat MG Loadout"}},
			};
			static const char* getaway_vehicles[5][4] = {
				{"Issi Classic", "Asbo", "Kanjo", "Sentinel Classic"},
				{"Retinue MK II", "Drift Yosemite", "Sugoi", "Jugular"},
				{"Sultan Classic", "Gauntlet Classic", "Ellie", "Komoda"},
				{"Manchez", "Stryder", "Defiler", "Lectro"},
				{"Zhaba", "Vagrant", "Outlaw", "Everon"},
			};
			static int approach = get_stat_int("H3OPT_APPROACH");
			static int last_approach = get_stat_int("H3_LAST_APPROACH");
			static int hard_approach = get_stat_int("H3_HARD_APPROACH");
			static int target = get_stat_int("H3OPT_TARGET");
			static int gunman = get_stat_int("H3OPT_CREWWEAP");
			static int gun_loadout = get_stat_int("H3OPT_WEAPS");
			static int driver = get_stat_int("H3OPT_CREWDRIVER");
			static int getaway_vehicle = get_stat_int("H3OPT_VEHS");
			static int hacker = get_stat_int("H3OPT_CREWHACKER");
			static int mask = get_stat_int("H3OPT_MASKS");

			last_approach = std::clamp(get_stat_int("H3_LAST_APPROACH"), 0, 3);
			if (approach >= 1 && approach <= 3 && approach == last_approach)
			{
				approach = approach % 3 + 1;
				set_stat_int("H3OPT_APPROACH", approach);
			}

			if (ImGui::Combo("Approach", &approach, approaches, std::size(approaches)))
				set_stat_int("H3OPT_APPROACH", approach);
			ImGui::Text("Last Approach: %s", approaches[last_approach]);
			const auto current_hard_approach = get_stat_int("H3_HARD_APPROACH");
			const auto current_approach = get_stat_int("H3OPT_APPROACH");
			ImGui::Text("Current Difficulty: %s", current_approach != 0 && current_approach == current_hard_approach ? "Hard" : "Normal");
			if (ImGui::Combo("Hard Approach", &hard_approach, approaches, std::size(approaches)))
				set_stat_int("H3_HARD_APPROACH", hard_approach);
			if (ImGui::Combo("Target", &target, targets, std::size(targets)))
				set_stat_int("H3OPT_TARGET", target);
			if (ImGui::Combo("Crew Gunman", &gunman, gunmen, std::size(gunmen)))
				set_stat_int("H3OPT_CREWWEAP", gunman);

			const auto selected_gunman = std::clamp(gunman - 1, 0, 4);
			const auto selected_approach = std::clamp(approach, 0, 3);
			if (ImGui::Combo("Gunman Weapon Loadout", &gun_loadout, gun_loadouts[selected_gunman][selected_approach], 2))
				set_stat_int("H3OPT_WEAPS", gun_loadout);
			if (ImGui::Combo("Crew Driver", &driver, drivers, std::size(drivers)))
				set_stat_int("H3OPT_CREWDRIVER", driver);

			const auto selected_driver = std::clamp(driver - 1, 0, 4);
			if (ImGui::Combo("Getaway Vehicle", &getaway_vehicle, getaway_vehicles[selected_driver], 4))
				set_stat_int("H3OPT_VEHS", getaway_vehicle);
			if (ImGui::Combo("Crew Hacker", &hacker, hackers, std::size(hackers)))
				set_stat_int("H3OPT_CREWHACKER", hacker);
			if (ImGui::Combo("Crew Masks", &mask, masks, std::size(masks)))
				set_stat_int("H3OPT_MASKS", mask);

			if (ImGui::Button("Silent and Sneaky: Diamonds"))
			{
				set_stat_int("H3OPT_APPROACH", 1);
				set_stat_int("H3_LAST_APPROACH", 3);
				set_stat_int("H3OPT_TARGET", 3);
				set_stat_int("H3OPT_BITSET1", 127);
				set_stat_int("H3OPT_DISRUPTSHIP", 3);
				set_stat_int("H3OPT_KEYLEVELS", 2);
			}
			ImGui::SameLine();
			components::command_button<"objectivetp">({}, "VIEW_TELEPORT_OBJECTIVE"_T);
			ImGui::SameLine();
			components::command_button<"skipcutscene">();

			static std::array<int, 4> cuts{};
			draw_cuts(cuts, 1972483 + 1497 + 736 + 92 + 1, "casino_cut");

			if (ImGui::Button("Reload Planning Board"))
				set_local_int("gb_casino_heist_planning"_J, 217, 2);

			static bool auto_grabber = false;
			toggle_switch("Auto Grabber", &auto_grabber);
			if (auto_grabber)
			{
				const auto grab_state = gta_util::find_script_thread("fm_mission_controller"_J);
				if (grab_state)
				{
					const auto state = *script_local(grab_state->m_stack, 10311).as<int*>();
					if (state == 3)
						set_local_int("fm_mission_controller"_J, 10311, 4);
					else if (state == 4)
						set_local_float("fm_mission_controller"_J, 10311 + 14, 2.0F);
				}
			}

			if (ImGui::Button("Bypass Fingerprint Hack"))
				set_local_int("fm_mission_controller"_J, 54118, 5);
			ImGui::SameLine();
			if (ImGui::Button("Bypass Keypad Hack"))
				set_local_int("fm_mission_controller"_J, 55188, 5);
		}

		void draw_cayo_perico_heist()
		{
			components::sub_title("Cayo Perico Heist");
			ImGui::TextUnformatted(live_cooldown_status("cayo_perico").c_str());
			if (ImGui::Button("Complete Preps"))
			{
				set_stat_int("H4CNF_UNIFORM", -1);
				set_stat_int("H4CNF_GRAPPEL", -1);
				set_stat_int("H4CNF_TROJAN", 5);
				set_stat_int("H4CNF_WEP_DISRP", 3);
				set_stat_int("H4CNF_ARM_DISRP", 3);
				set_stat_int("H4CNF_HEL_DISRP", 3);
				set_stat_int("H4_PLAYTHROUGH_STATUS", 32);
				set_stat_int("H4CNF_BS_GEN", -1);
				set_stat_int("H4CNF_BS_ENTR", 63);
				set_stat_int("H4CNF_BS_ABIL", 63);
				set_stat_int("H4CNF_APPROACH", -1);
				set_local_int("heist_island_planning"_J, 1578, 2);
			}
			ImGui::SameLine();
			if (ImGui::Button("Reset Preps"))
			{
				set_stat_int("H4_PROGRESS", 0);
				set_stat_int("H4_MISSIONS", 0);
				set_stat_int("H4CNF_APPROACH", 0);
				set_stat_int("H4CNF_TARGET", -1);
				set_stat_int("H4CNF_BS_GEN", 0);
				set_stat_int("H4CNF_BS_ENTR", 0);
				set_stat_int("H4CNF_BS_ABIL", 0);
				set_stat_int("H4_PLAYTHROUGH_STATUS", 0);
				set_local_int("heist_island_planning"_J, 1578, 2);
			}
			ImGui::SameLine();
			components::command_button<"objectivetp">({}, "VIEW_TELEPORT_OBJECTIVE"_T);
			ImGui::SameLine();
			components::command_button<"skipcutscene">();
			ImGui::SameLine();
			components::command_button<"suicide">();
			ImGui::SameLine();
			components::command_button<"bringpv">();
			ImGui::SameLine();
			components::command_button<"pvtp">();
			ImGui::SameLine();
			components::command_button<"lastvehtp">();

			static const char* targets[] = {"Panther Statue", "Pink Diamond", "Madrazo Files", "Bearer Bonds", "Ruby Necklace", "Sinsimito Tequila"};
			static constexpr int target_values[] = {5, 3, 4, 2, 1, 0};
			static const char* island_loot[] = {"Cash", "Cocaine", "Weed"};
			static constexpr int island_loot_values[] = {0, 1, 2};
			static const char* compound_loot[] = {"Cash", "Gold", "Painting"};
			static constexpr int compound_loot_values[] = {0, 1, 2};
			static constexpr int weapon_values[] = {1, 2, 3, 4, 5};
			static const char* weapons[] = {"Aggressor", "Conspirator", "Crack Shot", "Saboteur", "Marksman"};
			static int target = 0;
			static int island_target = 0;
			static int compound_target = 0;
			static int weapon = 0;
			static bool setup_initialized = false;
			static bool override_difficulty = false;
			static bool hard_difficulty = false;
			if (!setup_initialized)
			{
				const auto current_target = get_stat_int("H4CNF_TARGET");
				for (int i = 0; i < std::size(target_values); ++i)
					if (target_values[i] == current_target)
						target = i;

				const auto current_weapon = get_stat_int("H4CNF_WEAPONS");
				for (int i = 0; i < std::size(weapon_values); ++i)
					if (weapon_values[i] == current_weapon)
						weapon = i;

				hard_difficulty = get_stat_int("H4_PROGRESS") == 131055;
				setup_initialized = true;
			}
			if (!override_difficulty)
				hard_difficulty = get_stat_int("H4_PROGRESS") == 131055;

			ImGui::Combo("Primary Target", &target, targets, std::size(targets));
			ImGui::Combo("Island Secondary Target", &island_target, island_loot, std::size(island_loot));
			ImGui::Combo("Compound Secondary Target", &compound_target, compound_loot, std::size(compound_loot));
			ImGui::Combo("Weapon Loadout", &weapon, weapons, std::size(weapons));

			static bool use_non_legit_preset = false;
			toggle_switch("Use Non-Legit Preset", &use_non_legit_preset);
			ImGui::Text("Current Difficulty: %s", get_stat_int("H4_PROGRESS") == 131055 ? "Hard" : "Normal");
			toggle_switch("Override Difficulty", &override_difficulty);
			if (override_difficulty)
				toggle_switch("Hard Mode", &hard_difficulty);
			if (ImGui::Button("Apply Setup"))
			{
				int island_cash = get_stat_int("H4LOOT_CASH_I");
				int island_cash_scoped = get_stat_int("H4LOOT_CASH_I_SCOPED");
				int compound_cash = get_stat_int("H4LOOT_CASH_C");
				int compound_cash_scoped = get_stat_int("H4LOOT_CASH_C_SCOPED");
				int island_coke = get_stat_int("H4LOOT_COKE_I");
				int island_coke_scoped = get_stat_int("H4LOOT_COKE_I_SCOPED");
				int compound_coke = get_stat_int("H4LOOT_COKE_C");
				int compound_coke_scoped = get_stat_int("H4LOOT_COKE_C_SCOPED");
				int island_gold = get_stat_int("H4LOOT_GOLD_I");
				int island_gold_scoped = get_stat_int("H4LOOT_GOLD_I_SCOPED");
				int compound_gold = get_stat_int("H4LOOT_GOLD_C");
				int compound_gold_scoped = get_stat_int("H4LOOT_GOLD_C_SCOPED");
				int island_weed = get_stat_int("H4LOOT_WEED_I");
				int island_weed_scoped = get_stat_int("H4LOOT_WEED_I_SCOPED");
				int compound_weed = get_stat_int("H4LOOT_WEED_C");
				int compound_weed_scoped = get_stat_int("H4LOOT_WEED_C_SCOPED");
				int painting = get_stat_int("H4LOOT_PAINT");
				int painting_scoped = get_stat_int("H4LOOT_PAINT_SCOPED");
				int cash_value = get_stat_int("H4LOOT_CASH_V");
				int coke_value = get_stat_int("H4LOOT_COKE_V");
				int gold_value = get_stat_int("H4LOOT_GOLD_V");
				int weed_value = get_stat_int("H4LOOT_WEED_V");
				int painting_value = get_stat_int("H4LOOT_PAINT_V");

				if (use_non_legit_preset)
				{
					island_cash = island_cash_scoped = island_loot_values[island_target] == 0 ? 16711680 : 0;
					island_coke = island_coke_scoped = island_loot_values[island_target] == 1 ? 255 : 0;
					island_weed = island_weed_scoped = island_loot_values[island_target] == 2 ? 65280 : 0;
					compound_cash = compound_cash_scoped = compound_loot_values[compound_target] == 0 ? 16711680 : 0;
					compound_gold = compound_gold_scoped = compound_loot_values[compound_target] == 1 ? 255 : 0;
					painting = painting_scoped = compound_loot_values[compound_target] == 2 ? 127 : 0;
					compound_coke = compound_coke_scoped = 0;
					island_gold = island_gold_scoped = 0;
					compound_weed = compound_weed_scoped = 0;
					cash_value = 83250;
					coke_value = 202500;
					gold_value = 333333;
					weed_value = 135000;
					painting_value = 180000;
				}

				set_stats({
					{"H4CNF_TARGET", target_values[target]},
					{"H4LOOT_CASH_I", island_cash},
					{"H4LOOT_CASH_I_SCOPED", island_cash_scoped},
					{"H4LOOT_CASH_C", compound_cash},
					{"H4LOOT_CASH_C_SCOPED", compound_cash_scoped},
					{"H4LOOT_COKE_I", island_coke},
					{"H4LOOT_COKE_I_SCOPED", island_coke_scoped},
					{"H4LOOT_COKE_C", compound_coke},
					{"H4LOOT_COKE_C_SCOPED", compound_coke_scoped},
					{"H4LOOT_GOLD_I", island_gold},
					{"H4LOOT_GOLD_I_SCOPED", island_gold_scoped},
					{"H4LOOT_GOLD_C", compound_gold},
					{"H4LOOT_GOLD_C_SCOPED", compound_gold_scoped},
					{"H4LOOT_WEED_I", island_weed},
					{"H4LOOT_WEED_I_SCOPED", island_weed_scoped},
					{"H4LOOT_WEED_C", compound_weed},
					{"H4LOOT_WEED_C_SCOPED", compound_weed_scoped},
					{"H4LOOT_PAINT", painting},
					{"H4LOOT_PAINT_SCOPED", painting_scoped},
					{"H4LOOT_CASH_V", cash_value},
					{"H4LOOT_COKE_V", coke_value},
					{"H4LOOT_GOLD_V", gold_value},
					{"H4LOOT_WEED_V", weed_value},
					{"H4LOOT_PAINT_V", painting_value},
					{"H4CNF_BS_GEN", 262143},
					{"H4CNF_BS_ENTR", 63},
					{"H4CNF_BS_ABIL", 63},
					{"H4CNF_WEP_DISRP", 3},
					{"H4CNF_ARM_DISRP", 3},
					{"H4CNF_HEL_DISRP", 3},
					{"H4CNF_APPROACH", -1},
					{"H4CNF_BOLTCUT", 4424},
					{"H4CNF_UNIFORM", 5256},
					{"H4CNF_GRAPPEL", 5156},
					{"H4_MISSIONS", 65535},// -1
					{"H4CNF_WEAPONS", weapon_values[weapon]},
					{"H4CNF_TROJAN", 5},
					{"H4_PLAYTHROUGH_STATUS", 100},
				});
				if (override_difficulty)
					set_stat_int("H4_PROGRESS", hard_difficulty ? 131055 : 126823);
			}

			static std::array<int, 4> cuts{};
			draw_cuts(cuts, 1979291 + 831 + 56 + 1, "cayo_cut");

			if (ImGui::Button("Reload Planning Screen"))
				set_local_int("heist_island_planning"_J, 1578, 2);
			ImGui::SameLine();
			if (ImGui::Button("Bypass Drainage Cut"))
				set_local_int("fm_mission_controller_2020"_J, 31109, 6);
			ImGui::SameLine();
			if (ImGui::Button("Bypass Fingerprint"))
				set_local_int("fm_mission_controller_2020"_J, 26217, 5);
			ImGui::SameLine();
			if (ImGui::Button("Bypass Plasma Cutter"))
				set_local_float("fm_mission_controller_2020"_J, 32349 + 3, 100.0F);

		}

		void draw_contract()
		{
			components::sub_title("The Contract");
			if (ImGui::Button("Complete Preps"))
			{
				set_stat_int("FIXER_GENERAL_BS", -1);
				set_stat_int("FIXER_COMPLETED_BS", -1);
				set_stat_int("FIXER_STORY_COOLDOWN", -1);
			}
		}

		void draw_auto_shop()
		{
			components::sub_title("Auto Shop Contracts");
			if (ImGui::Button("Complete Preps"))
			{
				set_stat_int("TUNER_GEN_BS", 12543);
				set_local_int("tuner_planning"_J, 416, 2);
			}
			ImGui::SameLine();
			if (ImGui::Button("Reset Preps"))
			{
				set_stat_int("TUNER_GEN_BS", 12467);
				set_local_int("tuner_planning"_J, 416, 2);
			}
			if (ImGui::Button("Reload Board"))
				set_local_int("tuner_planning"_J, 416, 2);
		}

		void draw_salvage_yard()
		{
			components::sub_title("Salvage Yard");
			if (ImGui::Button("Complete Preps"))
			{
				set_stat_int("SALV23_GEN_BS", -1);
				set_stat_int("SALV23_SCOPE_BS", -1);
				set_stat_int("SALV23_FM_PROG", -1);
				set_stat_int("SALV23_INST_PROG", -1);
				set_local_int("vehrob_planning"_J, 545, 2);
			}
			ImGui::SameLine();
			if (ImGui::Button("Reset Preps"))
			{
				set_stat_int("SALV23_GEN_BS", 0);
				set_stat_int("SALV23_SCOPE_BS", 0);
				set_stat_int("SALV23_FM_PROG", 0);
				set_stat_int("SALV23_INST_PROG", 0);
			}
			if (ImGui::Button("Reload Board"))
				set_local_int("vehrob_planning"_J, 545, 2);
			if (ImGui::Button("Skip Weekly Cooldown"))
				set_stat_int("SALV23_WEEK_SYNC", 1);
		}

		void draw_cluckin_bell()
		{
			components::sub_title("Cluckin' Bell Farm Raid");
			if (ImGui::Button("Complete Preps"))
				set_stat_int("SALV23_INST_PROG", 31);
			ImGui::SameLine();
			if (ImGui::Button("Reset Preps"))
				set_stat_int("SALV23_INST_PROG", 0);
		}

		void draw_heist_database()
		{
			components::sub_title("Heist Database");
			static int selected_heist = 0;
			static std::size_t selected_entry = 0;
			static const char* heists[] = {"Diamond Casino Heist", "Cayo Perico Heist"};
			static char notes_buffer[512] = {};

			ImGui::Combo("Heist", &selected_heist, heists, std::size(heists));
			if (ImGui::InputTextMultiline("Notes / Reason", notes_buffer, std::size(notes_buffer), {0.0F, 80.0F}))
			{
				// The text field is part of the same menu typing lifecycle as the normal components.
			}
			if (ImGui::IsItemActive())
			{
				g.self.typing = TYPING_TICKS;
				draw_input_method_editor();
				*g_pointers->m_gta.m_allow_keyboard_layout_change = true;
			}
			if (ImGui::IsItemDeactivated())
				*g_pointers->m_gta.m_allow_keyboard_layout_change = false;
			if (ImGui::Button("Save Current Setup"))
			{
				if (selected_heist == 0)
					selected_entry = save_heist_database_entry("diamond_casino", casino_database_stats, notes_buffer);
				else
					selected_entry = save_heist_database_entry("cayo_perico", cayo_database_stats, notes_buffer);
				
				std::memset(notes_buffer, 0, sizeof(notes_buffer));
				ImGui::ClearActiveID();
			}
			ImGui::SameLine();
			if (ImGui::Button("Load Saved Setup"))
			{
				if (selected_heist == 0)
					load_heist_database_entry("diamond_casino", casino_database_stats, selected_entry);
				else
					load_heist_database_entry("cayo_perico", cayo_database_stats, selected_entry);
			}
			ImGui::SameLine();
			if (ImGui::Button("Delete Selected Entry"))
			{
				if (selected_heist == 0)
					delete_heist_database_entry("diamond_casino", casino_database_stats, selected_entry);
				else
					delete_heist_database_entry("cayo_perico", cayo_database_stats, selected_entry);
				if (selected_entry > 0)
					--selected_entry;
			}
			if (selected_heist == 0)
				draw_heist_database_entries("diamond_casino", casino_database_stats, selected_entry);
			else
				draw_heist_database_entries("cayo_perico", cayo_database_stats, selected_entry);
		}
	}

	enum class HeistType
	{
		APARTMENT_HEISTS,
		DOOMSDAY_HEIST,
		DIAMOND_CASINO_HEIST,
		CAYO_PERICO_HEIST,
		THE_CONTRACT,
		AUTO_SHOP,
		SALVAGE_YARD,
		CLUCKIN_BELL_FARM_RAID,
		HEIST_DATABASE
	};

	static void tab_item_heist(const char* label, const HeistType heist_type)
	{
		if (ImGui::BeginTabItem(label))
		{
			switch (heist_type)
			{
			case HeistType::APARTMENT_HEISTS: draw_apartment_heist(); break;
			case HeistType::DOOMSDAY_HEIST: draw_doomsday_heist(); break;
			case HeistType::DIAMOND_CASINO_HEIST: draw_casino_heist(); break;
			case HeistType::CAYO_PERICO_HEIST: draw_cayo_perico_heist(); break;
			case HeistType::THE_CONTRACT: draw_contract(); break;
			case HeistType::AUTO_SHOP: draw_auto_shop(); break;
			case HeistType::SALVAGE_YARD: draw_salvage_yard(); break;
			case HeistType::CLUCKIN_BELL_FARM_RAID: draw_cluckin_bell(); break;
			case HeistType::HEIST_DATABASE: draw_heist_database(); break;
			}

			ImGui::EndTabItem();
		}
	}

	void view::heist_editor()
	{
		if (!*g_pointers->m_gta.m_is_session_started)
		{
			ImGui::Text("NOT_ONLINE"_T.data());
			return;
		}
		
		if (ImGui::BeginTabBar("##heist_editor_tab_bar"))
		{
			tab_item_heist("GUI_TAB_APARTMENT_HEIST"_T.data(), HeistType::APARTMENT_HEISTS);
			tab_item_heist("GUI_TAB_DOOMSDAY_HEIST"_T.data(), HeistType::DOOMSDAY_HEIST);
			tab_item_heist("GUI_TAB_DIAMOND_CASINO_HEIST"_T.data(), HeistType::DIAMOND_CASINO_HEIST);
			tab_item_heist("GUI_TAB_CAYO_PERICO_HEIST"_T.data(), HeistType::CAYO_PERICO_HEIST);
			tab_item_heist("GUI_TAB_THE_CONTRACT"_T.data(), HeistType::THE_CONTRACT);
			tab_item_heist("GUI_TAB_AUTO_SHOP"_T.data(), HeistType::AUTO_SHOP);
			tab_item_heist("GUI_TAB_SALVAGE_YARD"_T.data(), HeistType::SALVAGE_YARD);
			tab_item_heist("GUI_TAB_CLUCKIN_BELL_FARM_RAID"_T.data(), HeistType::CLUCKIN_BELL_FARM_RAID);
			tab_item_heist("GUI_TAB_HEIST_DATABASE"_T.data(), HeistType::HEIST_DATABASE);
			ImGui::EndTabBar();
		}
	}
}
