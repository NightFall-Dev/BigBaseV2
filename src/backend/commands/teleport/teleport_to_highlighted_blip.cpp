#include "backend/command.hpp"
#include "natives.hpp"
#include "util/teleport.hpp"

namespace big
{
	class teleport_to_highlighted_blip : command
	{
		using command::command;

		virtual void execute(const command_arguments& args, const std::shared_ptr<command_context> ctx) override
		{
			if (args.size() == 1)
			{
				teleport::to_blip_id(static_cast<Blip>(args.get<uint64_t>(0)));
				return;
			}

			if (!args.size())
			{
				teleport::to_highlighted_blip();
				return;
			}

			ctx->report_error("highlighttp accepts either no arguments or one blip ID.");
		}
	};

	teleport_to_highlighted_blip g_teleport_to_highlighted_blip("highlighttp", "VIEW_HOTKEY_SETTINGS_TELEPORT_TO_SELECTED", "BACKEND_TELEPORT_TO_HIGHLIGHTED_BLIP_DESC", std::nullopt);
}