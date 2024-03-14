#include "pch.h"
#include "ChatLogger.h"

std::string ChatLogger::userName = "?";
std::string ChatLogger::userUID = "?";
std::string ChatLogger::userPlatform = "?";
std::string ChatLogger::currentMonth = "?";
std::filesystem::path ChatLogger::bmDataFolderFilePath;

void ChatLogger::logChat(PriWrapper pri, std::string chat, std::string quickChat) {
    // handle disabled quick chats
    bool isQuickChat = !(quickChat == "");
    bool quickChatsEnabled = cvarManager->getCvar("ChatLogger_quickChatsEnabled").getBoolValue();
    if (isQuickChat && !quickChatsEnabled) {
        LOG("Didn't update chat log... (quick chat logging is disabled)");
        return;
    }

    bool isUser = false;
    bool isTeammate = false;
    bool isOpponent = false;
    bool isSpectator = pri.IsSpectator();
    bool isBeingSpectated = false;

    bool userChatsEnabled = cvarManager->getCvar("ChatLogger_userChatsEnabled").getBoolValue();
    bool teammateChatsEnabled = cvarManager->getCvar("ChatLogger_teammateChatsEnabled").getBoolValue();
    bool opponentChatsEnabled = cvarManager->getCvar("ChatLogger_opponentChatsEnabled").getBoolValue();
    bool spectatorChatsEnabled = cvarManager->getCvar("ChatLogger_spectatorChatsEnabled").getBoolValue();
    bool spectatedPlayerChatsEnabled = cvarManager->getCvar("ChatLogger_spectatedPlayerChatsEnabled").getBoolValue();

    std::string dateAndTime = getCurrentTimeAndDate();
    std::string chatterUID = pri.GetUniqueIdWrapper().GetIdString();
    std::string relation;

    // handle disabled chats for user
    if (chatterUID == userUID) {
        isUser = true;
        relation = "me";
    }
    if (isUser && !userChatsEnabled) {
        LOG("Didn't update chat log... (user chat logging is disabled)");
        return;
    }

    if (isSpectator && !isUser) {
        relation = "spectator";
    }

    if (!isUser && !isSpectator) {
        // find chatter's team
        unsigned char chatterTeam = pri.GetTeamNum2();

        // find user's team
        PlayerControllerWrapper userPlayerController = gameWrapper->GetPlayerController();
        if (!userPlayerController) { return; }
        PriWrapper userPri = userPlayerController.GetPRI();
        if (!userPri) { return; }
        unsigned char usersTeam = userPri.GetTeamNum2();     // user's team is 255 if theyre spectating (unsigned char converts -1 to 255)

        // determine chatter's relation to user
        if (usersTeam != 255) {
            if (chatterTeam == usersTeam) {
                isTeammate = true;
                relation = "teammate";
            }
            else {
                isOpponent = true;
                relation = "opponent";
            }
        }
        else {
            isBeingSpectated = true;
            relation = "spectated by me";
        }
    }

    // handle disabled chats for spectator/teammate/opponent
    if (isSpectator && !spectatorChatsEnabled && !isUser) {
        LOG("Didn't update chat log... (spectator chat logging is disabled)");
        return;
    }
    if (isTeammate && !teammateChatsEnabled) {
        LOG("Didn't update chat log... (teammate chat logging is disabled)");
        return;
    }
    if (isOpponent && !opponentChatsEnabled) {
        LOG("Didn't update chat log... (opponent chat logging is disabled)");
        return;
    }
    if (isBeingSpectated && !spectatedPlayerChatsEnabled) {
        LOG("Didn't update chat log... (spectated player chat logging is disabled)");
        return;
    }

    // get additional info to be logged
    std::string chatterName = pri.GetPlayerName().ToString();
    chat = isQuickChat ? quickChat : chat;  // makes sure quickchat is parsed before logging

    OnlinePlatform platform = pri.GetPlatform();
    if (!platform) { return; }
    std::string platformStr = parsePlatform(platform);

    // console log the chat which will be saved
    LOG("\t\t{}", dateAndTime);
    LOG("Player:\t\t{}\t({})", chatterName, relation);
    LOG("Chat:\t\t\"{}\"", chat);

    // log chat in JSON file
    createJsonDataAndWriteToFile(chat, chatterName, relation, dateAndTime, platformStr);
}

void ChatLogger::logPartyChat(std::string name, std::string chat, bool isUser) {
    // handle disabled chats
    bool userChatsEnabled = cvarManager->getCvar("ChatLogger_userChatsEnabled").getBoolValue();
    if (isUser && !userChatsEnabled) {
        LOG("Didn't update chat log... (user chat logging is disabled)");
        return;
    }
    bool partyChatsEnabled = cvarManager->getCvar("ChatLogger_partyChatsEnabled").getBoolValue();
    if (!isUser && !partyChatsEnabled) {
        LOG("Didn't update chat log... (party chat logging is disabled)");
        return;
    }

    // set additional data
    std::string relation = isUser ? "me" : "party";
    std::string dateAndTime = getCurrentTimeAndDate();
    std::string platform = isUser ? userPlatform : "N/A";
    name = isUser ? userName : name;

    // console log the chat which will be saved
    LOG("\t\t" + dateAndTime);
    LOG("Player:\t\t" + name + "\t(" + relation + ")");
    LOG("Chat:\t\t\"" + chat + "\"");

    // log chat in JSON file
    createJsonDataAndWriteToFile(chat, name, relation, dateAndTime, platform);
}

std::string ChatLogger::parsePlatform(OnlinePlatform platform) {
    switch (platform)
    {
    case OnlinePlatform_Steam:
        return "Steam";
    case OnlinePlatform_PS4:
    case OnlinePlatform_PS3:
        return "PlayStation";
    case OnlinePlatform_Dingo:
        return "Xbox";
    case OnlinePlatform_OldNNX:
    case OnlinePlatform_NNX:
        return "Nintendo";
    case OnlinePlatform_PsyNet:
    case OnlinePlatform_Epic:
        return "Epic";
    default:
    case OnlinePlatform_QQ:
    case OnlinePlatform_Deleted:
    case OnlinePlatform_WeGame:
    case OnlinePlatform_MAX:
        return "Unknown";
    }
}

void ChatLogger::createJsonDataAndWriteToFile(std::string chat, std::string name, std::string relation, std::string time, std::string platform) {
    bool minifyChatLog = cvarManager->getCvar("ChatLogger_minifyChatLog").getBoolValue();

    // create new json object and populate it with data from chat
    nlohmann::json chatObj;
    chatObj["chat"] = chat;
    chatObj["playerName"] = name;
    chatObj["platform"] = platform;
    chatObj["relation"] = relation;
    chatObj["time"] = time;

    std::filesystem::path chatLogFilePath = bmDataFolderFilePath / "Chat Logs" / "ChatLog.json";
    std::string jsonFileRawStr = ReadContent(chatLogFilePath);

    // prevent crash on reading invalid JSON data
    try {
        auto chatJsonData = nlohmann::json::parse(jsonFileRawStr);
        chatJsonData["chatMessages"].push_back(chatObj);

        WriteContent(chatLogFilePath, chatJsonData.dump(minifyChatLog ? -1 : 4));
        LOG("Added to chat log :)");
    }
    catch (...) {
        LOG("*** Couldn't read the 'ChatLog.json' file! Make sure it contains valid JSON... ***");
    }
}

void ChatLogger::saveChatsAndClearLog() {
    // set filepath variables
    std::filesystem::path chatLogFilePath = bmDataFolderFilePath / "Chat Logs" / "ChatLog.json";
    std::filesystem::path savedChatsFilePath = bmDataFolderFilePath / "Chat Logs" / (currentMonth + "_SavedChats.json");

    // read JSON data as serialized string
    std::string chatLogFileRawStr = ReadContent(chatLogFilePath);
    std::string savedChatsFileRawStr = ReadContent(savedChatsFilePath);

    // prevent crash if JSON data is invalid
    try {
        auto chatLogJsonData = nlohmann::json::parse(chatLogFileRawStr);
        if (chatLogJsonData["chatMessages"].size() < 1) { return; }
        auto savedChatsJsonData = nlohmann::json::parse(savedChatsFileRawStr);

        // append chats from 'ChatLog.json' to existing chats in 'SavedChats.json'
        for (int i = 0; i < chatLogJsonData["chatMessages"].size(); i++) {
            savedChatsJsonData["chatMessages"].push_back(chatLogJsonData["chatMessages"][i]);
        }

        // write updated JSON data to file
        bool minifySavedChats = cvarManager->getCvar("ChatLogger_minifySavedChats").getBoolValue();
        WriteContent(savedChatsFilePath, savedChatsJsonData.dump(minifySavedChats ? -1 : 4));
        LOG("Saved chats to '{}_SavedChats.json' :)", currentMonth);

        clearLog("ChatLog.json");
    }
    catch (...) {
        LOG("*** Couln't read JSON file! Make sure it's formatted correctly... ***");
    }
}

void ChatLogger::clearLog(std::string fileName) {
    try {
        std::string playerName = userName;
        std::filesystem::path jsonFilePath = bmDataFolderFilePath / "Chat Logs" / fileName;
        if (std::filesystem::exists(jsonFilePath)) {
            std::ofstream NewFile(jsonFilePath);
            NewFile << "{ \"user\": \"" + playerName + "\", \"chatMessages\":[]}";
            NewFile.close();
            LOG("Cleared '{}' :)", fileName);
        }
        else {
            LOG("*** Couldn't clear '{}' ... it doesn't exist! ***", fileName);
        }
    }
    catch (...) {
        LOG("*** Something went wrong while clearing '{}' ***", fileName);
    }
}

std::string ChatLogger::getCurrentTimeAndDate(std::string format) {
    auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
    return format == "full" ? std::format("{:%m/%d/%Y %r}", time) : std::format("{:%B_%Y}", time);
}

std::string ChatLogger::ReadContent(std::filesystem::path FileName) {
    std::ifstream Temp(FileName);
    std::stringstream Buffer;
    Buffer << Temp.rdbuf();
    return (Buffer.str());
}

void ChatLogger::WriteContent(std::filesystem::path FileName, std::string Buffer) {
    std::ofstream File(FileName, std::ofstream::trunc);
    File << Buffer;
    File.close();
}

void ChatLogger::CheckJsonFiles() {
    std::filesystem::path chatLogsFolder = bmDataFolderFilePath / "Chat Logs";
    std::filesystem::path chatLogFilePath = chatLogsFolder / "ChatLog.json";
    std::filesystem::path savedChatsFilePath = chatLogsFolder / (currentMonth + "_SavedChats.json");

    // create 'Chat Logs' folder if it doesn't exist
    if (!std::filesystem::exists(chatLogsFolder)) {
        std::filesystem::create_directory(chatLogsFolder);
        LOG("'Chat Logs' folder didn't exist... so I created it.");
    }

    // create JSON files if they don't exist
    if (!std::filesystem::exists(chatLogFilePath)) {
        std::ofstream NewFile(chatLogFilePath);
        NewFile << "{ \"user\": \"" + userName + "\", \"chatMessages\":[]}";
        NewFile.close();
        LOG("'ChatLog.json' didn't exist... so I created it.");
    }
    if (!std::filesystem::exists(savedChatsFilePath)) {
        std::ofstream NewFile(savedChatsFilePath);
        NewFile << "{ \"user\": \"" + userName + "\", \"chatMessages\":[]}";
        NewFile.close();
        LOG("'{}_SavedChats.json' didn't exist... so I created it.", currentMonth);
    }
}

void ChatLogger::retrieveUserData() {
    try {
        bool isEpic = gameWrapper->IsUsingEpicVersion();
        userPlatform = isEpic ? "Epic" : "Steam";
        userName = gameWrapper->GetPlayerName().ToString();
        userUID = gameWrapper->GetUniqueID().GetIdString();
        bmDataFolderFilePath = gameWrapper->GetDataFolder();
        currentMonth = getCurrentTimeAndDate("month");
        LOG("Successfully retrieved user data");
        LOG("Player name: {}", userName);
        LOG("Player UID: {}", userUID);
        LOG("Platform: {}", userPlatform);
    }
    catch (...) {
        LOG("Couldn't retrieve user data :(");
    }
}