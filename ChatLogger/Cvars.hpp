#pragma once


#define CVAR(name) "chatlogger_" name	// automatically apply prefix

namespace Cvars
{
	// bools
	constexpr const char* save_quickchats =					CVAR("save_quickchats");
	constexpr const char* save_user_chats =					CVAR("save_user_chats");
	constexpr const char* save_party_chats =				CVAR("save_party_chats");
	constexpr const char* save_teammate_chats =				CVAR("save_teammate_chats");
	constexpr const char* save_opponent_chats =				CVAR("save_opponent_chats");
	constexpr const char* save_spectator_chats =			CVAR("save_spectator_chats");
	constexpr const char* save_spectated_player_chats =		CVAR("save_spectated_player_chats");

	constexpr const char* save_chats_and_clear_log =		CVAR("save_chats_and_clear_log");
	constexpr const char* clear_log =						CVAR("clear_log");

	constexpr const char* minify_saved_chats =				CVAR("minify_saved_chats");
	constexpr const char* minify_chat_log =					CVAR("minify_chat_log");
}


namespace Commands
{
	constexpr const char* clear_chat_log =					CVAR("clear_chat_log");
	constexpr const char* clear_saved_chats =				CVAR("clear_saved_chats");
}


#undef CVAR