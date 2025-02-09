#include "pch.h"
#include "ChatLogger.h"



// can be used to handle all chats.... in a future refactor
void ChatLogger::event_GFxData_Chat_TA_AddChatMessage(ActorWrapper caller, void* params, std::string event)
{
	auto chat_params = reinterpret_cast<FGFxChatMessage*>(caller.memory_address);
	if (!chat_params) return;

	// ...
}



// --------------------------------------- og callbacks -----------------------------------------

void ChatLogger::event_GFxData_Chat_TA_OnChatMessage(ActorWrapper caller, void* params, std::string event)
{
	const OnChatMessage_Params* chat_params = reinterpret_cast<OnChatMessage_Params*>(params);
	if (!chat_params) return;

	if (chat_params->ChatChannel != 2) return;	// skip if not a party chat

	if (!successfully_stored_user_info)
	{
		get_user_data();
		if (!successfully_stored_user_info) return;
	}

	std::string name = chat_params->PlayerName.to_wrapper().ToString();
	if (name.empty()) return;

	log_party_chat(chat_params);
}


void ChatLogger::event_ChatMessage_TA(ActorWrapper caller, void* params, std::string event)
{
	const ChatMessage_TA_Params* chat_params = reinterpret_cast<ChatMessage_TA_Params*>(params);
	if (!chat_params) return;

	if (!successfully_stored_user_info)
	{
		get_user_data();
		if (!successfully_stored_user_info) return;
	}

	PriWrapper pri(reinterpret_cast<uintptr_t>(chat_params->InPRI));
	if (!pri)
	{
		LOG("pri is NULL!");
		return;
	}

	std::string message = chat_params->Message.to_wrapper().ToString();

	log_chat(pri, message, chat_params->bPreset);
}


void ChatLogger::event_EventPreLoadMap(std::string event)
{
	if (!successfully_stored_user_info)
	{
		get_user_data();
		if (!successfully_stored_user_info) return;
	}

	bool saveChatsAndClearLogEnabled = cvarManager->getCvar(Cvars::save_chats_and_clear_log).getBoolValue();
	bool clearLogEnabled = cvarManager->getCvar(Cvars::clear_log).getBoolValue();

	if (saveChatsAndClearLogEnabled)
	{
		save_chats_and_clear_log();
	}
	else if (clearLogEnabled)
	{
		clear_log("ChatLog.json");
	}
}
