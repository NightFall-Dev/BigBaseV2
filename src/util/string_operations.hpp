#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace big::string::operations
{
	inline std::string base64_encode(const std::string& data)
	{
		static constexpr char sEncodingTable[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

		size_t in_len = data.size();
		size_t out_len = 4 * ((in_len + 2) / 3);
		std::string ret(out_len, '\0');
		size_t i;
		char* p = ret.data();

		for (i = 0; i < in_len - 2; i += 3)
		{
			*p++ = sEncodingTable[(data[i] >> 2) & 0x3F];
			*p++ = sEncodingTable[((data[i] & 0x3) << 4) | ((int)(data[i + 1] & 0xF0) >> 4)];
			*p++ = sEncodingTable[((data[i + 1] & 0xF) << 2) | ((int)(data[i + 2] & 0xC0) >> 6)];
			*p++ = sEncodingTable[data[i + 2] & 0x3F];
		}
		if (i < in_len)
		{
			*p++ = sEncodingTable[(data[i] >> 2) & 0x3F];
			if (i == (in_len - 1))
			{
				*p++ = sEncodingTable[((data[i] & 0x3) << 4)];
				*p++ = '=';
			}
			else
			{
				*p++ = sEncodingTable[((data[i] & 0x3) << 4) | ((int)(data[i + 1] & 0xF0) >> 4)];
				*p++ = sEncodingTable[((data[i + 1] & 0xF) << 2)];
			}
			*p++ = '=';
		}

		return ret;
	}

	inline int base64_value(char ch)
	{
		if (ch >= 'A' && ch <= 'Z')
			return ch - 'A';
		if (ch >= 'a' && ch <= 'z')
			return ch - 'a' + 26;
		if (ch >= '0' && ch <= '9')
			return ch - '0' + 52;
		if (ch == '+')
			return 62;
		if (ch == '/')
			return 63;
		return -1;
	}

	inline std::string base64_decode(const std::string& data)
	{
		std::string result;
		result.reserve((data.size() * 3) / 4);

		for (size_t i = 0; i < data.size();)
		{
			while (i < data.size() && (data[i] == '\n' || data[i] == '\r' || data[i] == ' ' || data[i] == '\t'))
				++i;
			if (i >= data.size())
				break;

			int c0 = base64_value(data[i++]);
			if (c0 < 0)
				break;
			int c1 = base64_value(data[i++]);
			if (c1 < 0)
				break;
			int c2 = base64_value(data[i++]);
			if (c2 < 0)
				break;
			int c3 = base64_value(data[i++]);
			if (c3 < 0)
				break;

			result.push_back(static_cast<char>((c0 << 2) | (c1 >> 4)));
			if (c2 >= 0)
				result.push_back(static_cast<char>(((c1 & 0xF) << 4) | (c2 >> 2)));
			if (c3 >= 0)
				result.push_back(static_cast<char>(((c2 & 0x3) << 6) | c3));
		}

		return result;
	}

	inline std::string to_lower(std::string& str)
	{
		std::string result = str;
		std::transform(result.begin(), result.end(), result.begin(), ::tolower);
		str = result;
		return result;
	}

	inline std::string to_upper(std::string& str)
	{
		std::string result = str;
		std::transform(result.begin(), result.end(), result.begin(), ::toupper);
		str = result;
		return result;
	}

	inline std::string trim(std::string& str)
	{
		std::string result = str;
		result.erase(result.begin(), std::find_if(result.begin(), result.end(), [](unsigned char ch) {
			return !std::isspace(ch);
		}));
		result.erase(std::find_if(result.rbegin(),
		                 result.rend(),
		                 [](unsigned char ch) {
			                 return !std::isspace(ch);
		                 })
		                 .base(),
		    result.end());
		str = result;
		return result;
	}

	inline std::string remove_whitespace(std::string& str)
	{
		std::string result = str;
		result.erase(std::remove_if(result.begin(), result.end(), isspace), result.end());
		str = result;
		return result;
	}

	static std::vector<std::string> split(const std::string text, char delimiter)
	{
		std::vector<std::string> tokens;
		std::size_t start = 0, end = 0;
		while ((end = text.find(delimiter, start)) != std::string::npos)
		{
			if (end != start)
			{
				tokens.push_back(text.substr(start, end - start));
			}
			start = end + 1;
		}
		if (end != start)
		{
			tokens.push_back(text.substr(start));
		}
		return tokens;
	}

	static std::string join(const std::vector<std::string>& tokens, char delimiter)
	{
		std::string result;
		for (size_t i = 0; i < tokens.size(); i++)
		{
			result += tokens[i];
			if (i != tokens.size() - 1)
			{
				result += delimiter;
			}
		}
		return result;
	}
}