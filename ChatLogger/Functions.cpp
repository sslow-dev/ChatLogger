#include "pch.h"
#include "ChatLogger.h"



void ChatLogger::log_chat(PriWrapper& pri, std::string& chat, bool is_quickchat)
{
	// handle disabled quick chats
	bool save_quickchats = cvarManager->getCvar(Cvars::save_quickchats).getBoolValue();
	if (is_quickchat && !save_quickchats)
	{
		LOG("Didn't update chat log... (quick chat logging is disabled)");
		return;
	}

	bool isUser = false;
	bool isTeammate = false;
	bool isOpponent = false;
	bool isSpectator = pri.IsSpectator();
	bool isBeingSpectated = false;

	bool save_user_chats =					cvarManager->getCvar(Cvars::save_user_chats).getBoolValue();
	bool save_teammate_chats =				cvarManager->getCvar(Cvars::save_teammate_chats).getBoolValue();
	bool save_opponent_chats =				cvarManager->getCvar(Cvars::save_opponent_chats).getBoolValue();
	bool save_spectator_chats =				cvarManager->getCvar(Cvars::save_spectator_chats).getBoolValue();
	bool save_spectated_player_chats =		cvarManager->getCvar(Cvars::save_spectated_player_chats).getBoolValue();

	SavedChatMessage chat_msg_data;

	chat_msg_data.time = get_timestamp();
	std::string chatter_uid = pri.GetUniqueIdWrapper().GetIdString();
	DEBUGLOG("chatter UID: {}", chatter_uid);
	DEBUGLOG("user UID: {}", users_uid);

	// handle disabled chats for user
	if (chatter_uid == users_uid)
	{
		isUser = true;
		chat_msg_data.relation = "me";
	}
	if (isUser && !save_user_chats)
	{
		LOG("Didn't update chat log... (user chat logging is disabled)");
		return;
	}

	if (isSpectator && !isUser)
	{
		chat_msg_data.relation = "spectator";
	}

	if (!isUser && !isSpectator)
	{
		// find chatter's team
		unsigned char chatterTeam = pri.GetTeamNum2();

		// find user's team
		PlayerControllerWrapper userPlayerController = gameWrapper->GetPlayerController();
		if (!userPlayerController) return;

		PriWrapper userPri = userPlayerController.GetPRI();
		if (!userPri) return;

		unsigned char usersTeam = userPri.GetTeamNum2();     // user's team is 255 if theyre spectating (unsigned char converts -1 to 255)

		// determine chatter's relation to user
		if (usersTeam != 255)
		{
			if (chatterTeam == usersTeam)
			{
				isTeammate = true;
				chat_msg_data.relation = "teammate";
			}
			else
			{
				isOpponent = true;
				chat_msg_data.relation = "opponent";
			}
		}
		else
		{
			isBeingSpectated = true;
			chat_msg_data.relation = "spectated by me";
		}
	}

	// handle disabled chats for spectator/teammate/opponent
	if (isSpectator && !save_spectator_chats && !isUser)
	{
		LOG("Didn't update chat log... (spectator chat logging is disabled)");
		return;
	}
	if (isTeammate && !save_teammate_chats)
	{
		LOG("Didn't update chat log... (teammate chat logging is disabled)");
		return;
	}
	if (isOpponent && !save_opponent_chats)
	{
		LOG("Didn't update chat log... (opponent chat logging is disabled)");
		return;
	}
	if (isBeingSpectated && !save_spectated_player_chats)
	{
		LOG("Didn't update chat log... (spectated player chat logging is disabled)");
		return;
	}

	// get additional info to be logged
	chat_msg_data.name =		pri.GetPlayerName().ToString();
	chat_msg_data.chat =		is_quickchat ? parse_quickchat(chat) : chat;
	chat_msg_data.platform =	parse_platform(static_cast<EOnlinePlatform>(static_cast<uint8_t>(pri.GetPlatform())));

	LOG("Logging chat from {} ...", chat_msg_data.name);
	write_chat_data_to_file(chat_msg_data);
}


void ChatLogger::log_party_chat(const OnChatMessage_Params* chat_params)
{
	if (!chat_params)
	{
		LOG("[ERROR] OnChatMessage_Params* is null");
		return;
	}

	// handle disabled chats
	bool save_user_chats = cvarManager->getCvar(Cvars::save_user_chats).getBoolValue();
	if (chat_params->bLocalPlayer && !save_user_chats)
	{
		LOG("Didn't update chat log... (user chat logging is disabled)");
		return;
	}
	bool save_party_chats = cvarManager->getCvar(Cvars::save_party_chats).getBoolValue();
	if (!chat_params->bLocalPlayer && !save_party_chats)
	{
		LOG("Didn't update chat log... (party chat logging is disabled)");
		return;
	}

	SavedChatMessage chat_msg_data;
	chat_msg_data.name =		chat_params->PlayerName.to_wrapper().ToString();
	chat_msg_data.chat =		chat_params->Message.to_wrapper().ToString();
	chat_msg_data.relation =	chat_params->bLocalPlayer ? "me" : "party";
	chat_msg_data.time =		get_timestamp();
	chat_msg_data.platform =	parse_platform(chat_params->SenderId.Platform);

	LOG("Logging chat from {} ...", chat_msg_data.name);
	write_chat_data_to_file(chat_msg_data);
}


void ChatLogger::write_chat_data_to_file(const SavedChatMessage& chat_msg_data)
{
	json chat_log_json = Files::get_json(chat_log_json_path);
	if (chat_log_json.empty()) return;

	// create new json object and populate it with data from chat
	json chatObj;
	chatObj["chat"] =			chat_msg_data.chat;
	chatObj["playerName"] =		chat_msg_data.name;
	chatObj["platform"] =		chat_msg_data.platform;
	chatObj["relation"] =		chat_msg_data.relation;
	chatObj["time"] =			chat_msg_data.time;

	chat_log_json["chatMessages"].push_back(chatObj);

	bool minify_chat_log = cvarManager->getCvar(Cvars::minify_chat_log).getBoolValue();
	Files::write_json(chat_log_json_path, chat_log_json, minify_chat_log ? -1 : 4);
}

void ChatLogger::save_chats_and_clear_log()
{
	constexpr const char* chat_msgs_key = "chatMessages";

	// get json data
	json chat_log_json =	Files::get_json(chat_log_json_path);
	json saved_chats_json =	Files::get_json(saved_chats_json_path);
	if (chat_log_json.empty() || saved_chats_json.empty()) return;

	if (!chat_log_json.contains(chat_msgs_key))
	{
		LOG("[ERROR] {} doesn't contain a \"{}\" array", chat_log_json_path.filename().string(), chat_msgs_key);
		return;
	}
	if (!saved_chats_json.contains(chat_msgs_key))
	{
		LOG("[ERROR] {} doesn't contain a \"{}\" array", saved_chats_json_path.filename().string(), chat_msgs_key);
		return;
	}

	auto& chat_log_msgs_arr = chat_log_json.at(chat_msgs_key);
	if (chat_log_msgs_arr.empty()) return;

	auto& saved_chats_msgs_arr = saved_chats_json.at(chat_msgs_key);	// it's fine if this is empty

	// append chats from 'ChatLog.json' to the end of existing chats in '...SavedChats.json'
	saved_chats_msgs_arr.insert(saved_chats_msgs_arr.end(), chat_log_msgs_arr.begin(), chat_log_msgs_arr.end());

	// write updated JSON data to file
	bool minify_saved_chats = cvarManager->getCvar(Cvars::minify_saved_chats).getBoolValue();

	Files::write_json(saved_chats_json_path, saved_chats_json, minify_saved_chats ? -1 : 4);
	LOG("Saved chats to '{}' :)", saved_chats_json_path.filename().string());

	clear_log(chat_log_json_path);
}

void ChatLogger::clear_log(const fs::path& file_path)
{
	write_empty_log_file(file_path);
	LOG("Cleared '{}' :)", file_path.filename().string());
}


void ChatLogger::write_empty_log_file(const fs::path& file_path)
{
	if (!successfully_stored_user_info)
	{
		get_user_data();
	}

	std::ofstream NewFile(file_path);
	NewFile << "{ \"user\": \"" + users_name + "\", \"chatMessages\":[]}";
	NewFile.close();
}


std::string ChatLogger::get_timestamp(bool only_month_and_year)
{
	auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
	return only_month_and_year ? std::format("{:%B_%Y}", time) : std::format("{:%m/%d/%Y %r}", time);
}


void ChatLogger::plugin_init()
{
	set_json_filepaths();
	get_user_data();
}


void ChatLogger::set_json_filepaths()
{
	fs::path chatLogsFolder = gameWrapper->GetDataFolder() / "Chat Logs";

	chat_log_json_path = chatLogsFolder / "ChatLog.json";
	saved_chats_json_path = chatLogsFolder / (get_timestamp(true) + "_SavedChats.json");

	// create 'Chat Logs' folder if it doesn't exist
	if (!std::filesystem::exists(chatLogsFolder))
	{
		std::filesystem::create_directory(chatLogsFolder);
		LOG("'Chat Logs' folder didn't exist... so I created it.");
	}
}


void ChatLogger::check_json_files()
{
	// create JSON files if they don't exist
	if (!std::filesystem::exists(chat_log_json_path))
	{
		write_empty_log_file(chat_log_json_path);
		LOG("'{}' didn't exist... so I created it.", chat_log_json_path.filename().string());
	}
	if (!std::filesystem::exists(saved_chats_json_path))
	{
		write_empty_log_file(saved_chats_json_path);
		LOG("'{}' didn't exist... so I created it.", saved_chats_json_path.filename().string());
	}
}


void ChatLogger::get_user_data()
{
	users_name =		gameWrapper->GetPlayerName().ToString();
	users_uid =			gameWrapper->GetUniqueID().GetIdString();

	if (users_name == "Player 1" || users_uid == "Unknown|0|0")
	{
		LOG("Unable to get valid user data...");
		return;
	}
	else
		successfully_stored_user_info = true;

	users_platform =	gameWrapper->IsUsingEpicVersion() ? "Epic" : "Steam";

	LOG("Successfully retrieved user data...");
	LOG("Player name: {}", users_name);
	LOG("Player UID: {}", users_uid);
	LOG("Platform: {}", users_platform);

	check_json_files();	// only create the json files when valid user data is stored
}


std::string ChatLogger::parse_quickchat(const std::string& msg)
{
	auto it = quickchat_ids_to_text.find(msg);
	if (it == quickchat_ids_to_text.end())
	{
		LOG("[ERROR] \"{}\" not found in hardcoded quickchat map... plugin might need an update", msg);
		return std::string();
	}

	return it->second;
}


std::string ChatLogger::parse_platform(EOnlinePlatform platform)
{
	switch (platform)
	{
	case EOnlinePlatform::OnlinePlatform_Steam:
		return "Steam";
	case EOnlinePlatform::OnlinePlatform_PS4:
	case EOnlinePlatform::OnlinePlatform_PS3:
		return "PlayStation";
	case EOnlinePlatform::OnlinePlatform_Dingo:
		return "Xbox";
	case EOnlinePlatform::OnlinePlatform_OldNNX:
	case EOnlinePlatform::OnlinePlatform_NNX:
		return "Nintendo";
	case EOnlinePlatform::OnlinePlatform_PsyNet:
	case EOnlinePlatform::OnlinePlatform_Epic:
		return "Epic";
	case EOnlinePlatform::OnlinePlatform_Unknown:
	case EOnlinePlatform::OnlinePlatform_QQ_DEPRECATED:
	case EOnlinePlatform::OnlinePlatform_WeGame_DEPRECATED:
	case EOnlinePlatform::OnlinePlatform_Deleted:
	case EOnlinePlatform::OnlinePlatform_END:
	default:
		return "Unknown";
	}
}
