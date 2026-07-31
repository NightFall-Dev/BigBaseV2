#include "backend/command.hpp"
#include "util/string_operations.hpp"

namespace big
{
	class recovery : command
	{
		using command::command;

		virtual CommandAccessLevel get_access_level() override
		{
			return CommandAccessLevel::NONE;
		}

		virtual void execute(const command_arguments&, const std::shared_ptr<command_context> ctx) override
		{
			auto msg = big::string::operations::base64_decode("TW9uZXkgYW5kIHJlY292ZXJ5IG9wdGlvbnMgYXJlIG5vdCBzdXBwb3J0ZWQgaW4gWWltTWVudSB0byBrZWVwIFJvY2tzdGFyL1Rha2UgVHdvIGhhcHB5LiBZb3UgY2FuIHRyeSBLaWRkaW9uJ3MgTW9kZXN0IE1lbnUgKGZyZWUpIGluc3RlYWQsIGJ1dCBtYWtlIHN1cmUgdG8gb25seSBnZXQgaXQgZnJvbSBVbmtub3duQ2hlYXRzLm1lLCB0aGUgcmVzdCBhcmUgc2NhbXMgYW5kIG1heSBjb250YWluIG1hbHdhcmU=");
			ctx->report_error(msg);
		}
	};

	recovery g_money("money", "", "", 0);
	recovery g_cash("cash", "", "", 0);
	recovery g_drop("drop", "", "", 0);
	recovery g_stats("stats", "", "", 0);
}