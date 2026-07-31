#include "api_service.hpp"

#include "http_client/http_client.hpp"


namespace big
{

	api_service::api_service()
	{
		g_api_service = this;
	}

	api_service::~api_service()
	{
		g_api_service = nullptr;
	}

	std::string api_service::get_translation(std::string message, std::string target_language)
	{
		std::string url = g.session.chat_translator.endpoint;
		const auto response = g_http_client.post(url,
		    {{"Content-Type", "application/json"}}, std::format(R"({{"q":"{}", "source":"auto", "target": "{}"}})", message, target_language));
		if (response.status_code == 200)
		{
			try
			{
				nlohmann::json obj = nlohmann::json::parse(response.text);
				std::string source_language = obj["detectedLanguage"]["language"];
				std::string result = obj["translatedText"];

				if (source_language == g.session.chat_translator.target_language && g.session.chat_translator.bypass_same_language)
					return "";
				return result;
			}
			catch (std::exception& e)
			{
				LOG(WARNING) << "[Chat Translator]Error when parse JSON data: " << e.what();

				return "";
			}
		}
		else if (response.status_code == 0)
		{
			g.session.chat_translator.enabled = false;
			g_notification_service.push_error("TRANSLATOR_TOGGLE"_T.data(), "TRANSLATOR_FAILED_TO_CONNECT"_T.data());
			LOG(WARNING) << "[Chat Translator]Unable to connect to LibreTranslate server. Follow the guide in Yimmenu Wiki to setup LibreTranslate server on your computer.";
		}
		else
		{
			LOG(WARNING) << "[Chat Translator]Error when sending request. Status code: " << response.status_code << " Response: " << response.text;
		}
		
		return "";
	}

	bool api_service::get_rid_from_username(std::string_view username, uint64_t& result)
	{
		const auto response = g_http_client.post("https://scui.rockstargames.com/api/friend/accountsearch", {{"Authorization", AUTHORIZATION_TICKET}, {"X-Requested-With", "XMLHttpRequest"}}, {std::format("searchNickname={}", username)});
		if (response.status_code == 200)
		{
			try
			{
				nlohmann::json obj = nlohmann::json::parse(response.text);

				if (obj["Total"] > 0 && username.compare(obj["Accounts"].at(0)["Nickname"]) == 0)
				{
					result = obj["Accounts"].at(0)["RockstarId"];
					return true;
				}
			}
			catch (std::exception& e)
			{
				return false;
			}
		}

		return false;
	}

	bool api_service::get_username_from_rid(uint64_t rid, std::string& result)
	{
		const auto response = g_http_client.post("https://scui.rockstargames.com/api/friend/getprofile", {{"Authorization", AUTHORIZATION_TICKET}, {"X-Requested-With", "XMLHttpRequest"}, {"Content-Type", "application/json"}}, std::format(R"({{"RockstarId":"{}"}})", rid));
		if (response.status_code == 200)
		{
			try
			{
				nlohmann::json obj = nlohmann::json::parse(response.text);
				result             = obj["Accounts"].at(0)["RockstarAccount"]["Name"];
				return true;
			}
			catch (std::exception& e)
			{
				return false;
			}
		}

		return false;
	}

	// Ratelimit: 10 per Minute, if exceeded than 5 min cooldown
	bool api_service::send_socialclub_message(uint64_t rid, std::string_view message)
	{
		const auto response = g_http_client.post("https://scui.rockstargames.com/api/messaging/sendmessage", {{"Authorization", AUTHORIZATION_TICKET}, {"X-Requested-With", "XMLHttpRequest"}, {"Content-Type", "application/json"}}, {std::format(R"({{"env":"prod","title":"gta5","version":11,"recipientRockstarId":"{}","messageText":"{}"}})", rid, message)});

		return response.status_code == 200;
	}
}