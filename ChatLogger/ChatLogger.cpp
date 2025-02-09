#include "pch.h"
#include "ChatLogger.h"


BAKKESMOD_PLUGIN(ChatLogger, "Chat Logger", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;


void ChatLogger::onLoad()
{
	_globalCvarManager = cvarManager;
	plugin_init();

	// register console variables and set default options
	cvarManager->registerCvar(Cvars::save_quickchats,					"0", "Toggle quick chat logging",				true, true, 0, true, 1);
	cvarManager->registerCvar(Cvars::save_user_chats,					"1", "Toggle user chat logging",				true, true, 0, true, 1);
	cvarManager->registerCvar(Cvars::save_party_chats,					"1", "Toggle party chat logging",				true, true, 0, true, 1);
	cvarManager->registerCvar(Cvars::save_teammate_chats,				"1", "Toggle teammate chat logging",			true, true, 0, true, 1);
	cvarManager->registerCvar(Cvars::save_opponent_chats,				"1", "Toggle opponent logging",					true, true, 0, true, 1);
	cvarManager->registerCvar(Cvars::save_spectator_chats,				"1", "Toggle spectator chat logging",			true, true, 0, true, 1);
	cvarManager->registerCvar(Cvars::save_spectated_player_chats,		"1", "Toggle spectated player chat logging",	true, true, 0, true, 1);

	cvarManager->registerCvar(Cvars::save_chats_and_clear_log,			"1", "Toggle saving Chats and clearing ChatLog.json after every match", false, true, 0, true, 1);
	cvarManager->registerCvar(Cvars::clear_log,							"0", "Toggle clearing 'Chats.json' after every match", false, true, 0, true, 1);

	cvarManager->registerCvar(Cvars::minify_saved_chats,				"0", "Toggle minification of JSON file to save space", true, true, 0, true, 1);
	cvarManager->registerCvar(Cvars::minify_chat_log,					"0", "Toggle minification of JSON file to save space", true, true, 0, true, 1);


	// register console commands
	auto clear_chat_log_cmd = [this](std::vector<std::string> args)
	{
		clear_log(chat_log_json_path);
	};
	cvarManager->registerNotifier(Commands::clear_chat_log, clear_chat_log_cmd, "clear 'ChatLog.json'", PERMISSION_ALL);


	auto clear_saved_chats_cmd = [this](std::vector<std::string> args)
	{
		clear_log(saved_chats_json_path);
	};
	cvarManager->registerNotifier(Commands::clear_saved_chats, clear_saved_chats_cmd, "clear saved chats JSON file", PERMISSION_ALL);



	// hook events
	gameWrapper->HookEventPost(Events::EventPreLoadMap, std::bind(&ChatLogger::event_EventPreLoadMap, this, std::placeholders::_1));

	//gameWrapper->HookEventWithCallerPost<ActorWrapper>(Events::GFxData_Chat_TA_AddChatMessage,
	//	std::bind(&ChatLogger::event_GFxData_Chat_TA_AddChatMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	gameWrapper->HookEventWithCallerPost<ActorWrapper>(Events::GFxData_Chat_TA_OnChatMessage,
		std::bind(&ChatLogger::event_GFxData_Chat_TA_OnChatMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	gameWrapper->HookEventWithCallerPost<ActorWrapper>(Events::ChatMessage_TA,
		std::bind(&ChatLogger::event_ChatMessage_TA, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));




	LOG("Chat Logger is loaded!");
}

