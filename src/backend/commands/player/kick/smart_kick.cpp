#include "backend/player_command.hpp"

namespace big
{
	class smart_kick : player_command
	{
		using player_command::player_command;

		virtual CommandAccessLevel get_access_level() override
		{
			return CommandAccessLevel::TOXIC;
		}

		virtual void execute(player_ptr player, const command_arguments& _args, const std::shared_ptr<command_context> ctx) override
		{
			if (g_player_service->get_self()->is_host())
			{
				player_command::get("breakup"_J)->call(player, {});
			}
			else
			{
				if (player->is_host())
				{
					constexpr size_t arg_count = 16;
					int64_t args[arg_count] = {(int64_t)eRemoteEvent::InteriorControl, (int64_t)self::id, 1 << player->id(), (int64_t)(int)-1};

					g_pointers->m_gta.m_trigger_script_event(1, args, arg_count, 1 << player->id(), (int)eRemoteEvent::InteriorControl);
				}
				else
				{
					player_command::get("desync"_J)->call(player, {});
				}
			}
		}
	};

	smart_kick g_smart_kick("smartkick", "SMART_KICK", "SMART_KICK_DESC", 0, false);
}
