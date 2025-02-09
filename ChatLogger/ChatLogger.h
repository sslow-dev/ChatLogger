#pragma once

#include "GuiBase.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include "Utils.hpp"
#include "Cvars.hpp"
#include "Events.hpp"
#include "GameStructs.hpp"

#include "version.h"
constexpr auto plugin_version =				stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);
constexpr auto plugin_version_no_build =	"v" stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH);



class ChatLogger : public BakkesMod::Plugin::BakkesModPlugin, public SettingsWindowBase
{
	void onLoad() override;

	bool successfully_stored_user_info = false;
	std::string users_name;
	std::string users_platform;
	std::string users_uid;

	fs::path saved_chats_json_path;
	fs::path chat_log_json_path;


	// on plugin load
	void plugin_init();
	void set_json_filepaths();
	void get_user_data();
	void check_json_files();


	// plugin functions
	void log_chat(PriWrapper& pri, std::string& chat, bool is_quickchat);
	void log_party_chat(const OnChatMessage_Params* chat_params);
	void write_chat_data_to_file(const SavedChatMessage& saved_chat_data);
	void save_chats_and_clear_log();
	void clear_log(const fs::path& file_path);
	void write_empty_log_file(const fs::path& file_path);

	static std::string parse_platform(EOnlinePlatform platform);
	static std::string parse_quickchat(const std::string& msg);
	static std::string get_timestamp(bool only_month_and_year = false);


	// hook callbacks
	void event_GFxData_Chat_TA_AddChatMessage(ActorWrapper caller, void* params, std::string event);
	void event_GFxData_Chat_TA_OnChatMessage(ActorWrapper caller, void* params, std::string event);
	void event_ChatMessage_TA(ActorWrapper caller, void* params, std::string event);
	void event_EventPreLoadMap(std::string event);

public:
	void RenderSettings() override;
};
