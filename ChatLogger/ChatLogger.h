#pragma once

#include "GuiBase.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include <nlohmann.hpp>
#include <chrono>
#include <format>
#include <iostream>
#include <fstream>
#include <filesystem>

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

const std::vector<std::vector<std::string>> QuickChatGroups = {
	{"I got it!","Need boost!","Take the shot!","Defending...","Go for it!","Centering!","All yours.","In position.","Incoming!","Faking.","Bumping!","On your left.","On your right.","Passing!"},
	{"Nice shot!","Great pass!","Thanks!","What a save!","Nice one!","What a play!","Great clear!","Nice block!","Nice bump!","Nice demo!"},
	{"OMG!","Noooo!","Wow!","Close one!","No way!","Holy cow!","Whew.","Siiiick!","Calculated.","Savage!","Okay."},
	{"$#@%!","No problem.","Whoops...","Sorry!","My bad...","Oops!","My fault."},
	{"gg","Well played.","That was fun!","Rematch!","One. More. Game.","What a game!","Nice moves.","Everybody dance!"},
	{"Good luck!","Have fun!", "Get ready.","This is Rocket League!","Let's do this!","Here. We. Go.","Nice cars!","I'll do my best."}
};

class ChatLogger : public BakkesMod::Plugin::BakkesModPlugin
	, public SettingsWindowBase
{
	void onLoad() override;
	void onChat(void* params);
	void logChat(PriWrapper pri, std::string messageContent, std::string quickChat);
	void logPartyChat(std::string name, std::string chat, bool isUser);
	void CheckJsonFiles();
	void createJsonDataAndWriteToFile(std::string chat, std::string name, std::string relation, std::string time, std::string platform);
	void saveChatsAndClearLog();
	void clearLog(std::string fileName);
	void retrieveUserData();
	void WriteContent(std::filesystem::path FileName, std::string Buffer);
	std::string ReadContent(std::filesystem::path FileName);
	std::string parsePlatform(OnlinePlatform platform);
	std::string getCurrentTimeAndDate(std::string format = "full");
	static std::string userName;
	static std::filesystem::path bmDataFolderFilePath;
	static std::string userPlatform;
	static std::string userUID;
	static std::string currentMonth;


public:
	void RenderSettings() override;
};
