#include "pch.h"
#include "ChatLogger.h"


BAKKESMOD_PLUGIN(ChatLogger, "Chat Logger", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

// ---------------------------------------- just for party chats -_- ------------------------------------------

std::string cleanWString(std::wstring wStr) {
    // turn non ASCII characters into '?'
    for (int i = 0; i < wStr.size(); i++) {
        if (wStr[i] > 127) {
            wStr[i] = 63;
        }
    }
    std::string cleanedStr(wStr.begin(), wStr.end());
    LOG("cleaned: " + cleanedStr);
    return cleanedStr;
}

struct FString
{
public:
    using ElementType = const wchar_t;
    using ElementPointer = ElementType*;

private:
    ElementPointer ArrayData;
    int32_t ArrayCount;
    int32_t ArrayMax;

public:
    FString()
    {
        ArrayData = nullptr;
        ArrayCount = 0;
        ArrayMax = 0;
    }

    FString(ElementPointer other)
    {
        ArrayData = nullptr;
        ArrayCount = 0;
        ArrayMax = 0;
        ArrayMax = ArrayCount = *other ? (wcslen(other) + 1) : 0;
        if (ArrayCount > 0)
        {
            ArrayData = other;
        }
    }

    ~FString() {}

public:
    std::string ToString() const
    {
        if (!IsValid())
        {
            std::wstring wideStr(ArrayData);
            std::string str(wideStr.begin(), wideStr.end());
            return str;
        }
        return std::string("lobby");
    }

    std::string ToCleanedString() const {
        if (!IsValid())
        {
            std::wstring wideStr(ArrayData);
            // turn unknown ASCII characters into '?'
            for (int i = 0; i < wideStr.size(); i++) {
                if (wideStr[i] > 127) {
                    wideStr[i] = 63;
                }
            }
            std::string cleanedStr(wideStr.begin(), wideStr.end());
            LOG("cleaned: " + cleanedStr);
            return cleanedStr;
        }
        return std::string("lobby");
    }

    bool IsValid() const
    {
        return !ArrayData;
    }

    FString operator=(ElementPointer other)
    {
        if (ArrayData != other)
        {
            ArrayMax = ArrayCount = *other ? (wcslen(other) + 1) : 0;
            if (ArrayCount > 0)
            {
                ArrayData = other;
            }
        }
        return *this;
    }
    bool operator==(const FString& other)
    {
        return (!wcscmp(ArrayData, other.ArrayData));
    }
};

FString FS(const std::string& s) {
    wchar_t* p = new wchar_t[s.size() + 1];
    for (std::string::size_type i = 0; i < s.size(); ++i)
        p[i] = s[i];
    p[s.size()] = '\0';
    return FString(p);
}

std::string Lower(std::string S) {
    for (auto& c : S) {
        c = tolower(c);
    }
    return S;
}

struct ChatMessage {
    int32_t Team;
    class FString PlayerName;
    class FString Message;
    uint8_t ChatChannel;
    bool bLocalPlayer : 1;
    unsigned char IDPadding[0x48];
    uint8_t MessageType;
};

// ------------------------------------------------------------------------------------------------------------

struct ChatMessageParams
{
    void* InPRI;
    uint8_t Message[0x10];
    uint8_t ChatChannel;
    bool bPreset;
};

void ChatLogger::onLoad()
{
    _globalCvarManager = cvarManager;
    retrieveUserData();
    CheckJsonFiles();

    // register console variables and set default options
    cvarManager->registerCvar("ChatLogger_quickChatsEnabled", "0", "Toggle quick chat logging", true, true, 0, true, 1);
    cvarManager->registerCvar("ChatLogger_userChatsEnabled", "1", "Toggle user chat logging", true, true, 0, true, 1);
    cvarManager->registerCvar("ChatLogger_partyChatsEnabled", "1", "Toggle party chat logging", true, true, 0, true, 1);
    cvarManager->registerCvar("ChatLogger_teammateChatsEnabled", "1", "Toggle teammate chat logging", true, true, 0, true, 1);
    cvarManager->registerCvar("ChatLogger_opponentChatsEnabled", "1", "Toggle opponent logging", true, true, 0, true, 1);
    cvarManager->registerCvar("ChatLogger_spectatorChatsEnabled", "1", "Toggle spectator chat logging", true, true, 0, true, 1);
    cvarManager->registerCvar("ChatLogger_spectatedPlayerChatsEnabled", "1", "Toggle spectator chat logging", true, true, 0, true, 1);

    cvarManager->registerCvar("ChatLogger_saveChatsAndClearLog", "1", "Toggle saving Chats and clearing ChatLog.json after every match", false, true, 0, true, 1);
    cvarManager->registerCvar("ChatLogger_clearLog", "0", "Toggle clearing 'Chats.json' after every match", false, true, 0, true, 1);

    cvarManager->registerCvar("ChatLogger_minifySavedChats", "0", "Toggle whether or not to minify JSON file to save space", true, true, 0, true, 1);
    cvarManager->registerCvar("ChatLogger_minifyChatLog", "0", "Toggle whether or not to minify JSON file to save space", true, true, 0, true, 1);

    // register console commands
    cvarManager->registerNotifier("ChatLogger_clearChatLog", [this](std::vector<std::string> args) {
        clearLog("ChatLog.json");
        }, "", PERMISSION_ALL);

    cvarManager->registerNotifier("ChatLogger_clearSavedChats", [this](std::vector<std::string> args) {
        clearLog(currentMonth + "_SavedChats.json");
        }, "", PERMISSION_ALL);

    // triggered on chats that aren't party chats or lobby notifications
    gameWrapper->HookEventWithCallerPost<PlayerControllerWrapper>("Function TAGame.PlayerController_TA.ChatMessage_TA",
        [this](PlayerControllerWrapper caller, void* params, std::string eventname) {
            onChat(params);
        });

    // triggered on all chats sent
    gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GFxData_Chat_TA.OnChatMessage", [this](ActorWrapper Caller, void* params, ...) {
        ChatMessage* Params = (ChatMessage*)params;
        uint8_t chatChannel = Params->ChatChannel;

        if (chatChannel == 2) {
            std::string CleanedPlayerName = Params->PlayerName.ToCleanedString();
            if (CleanedPlayerName == "lobby") { return; }
            std::string CleanedContent = Params->Message.ToCleanedString();
            bool fromUser = CleanedPlayerName == cleanWString(gameWrapper->GetPlayerName().ToWideString());

            logPartyChat(CleanedPlayerName, CleanedContent, fromUser);
        }
        });

    // when user leaves match
    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed", [this](std::string eventName) {
        bool saveChatsAndClearLogEnabled = cvarManager->getCvar("ChatLogger_saveChatsAndClearLog").getBoolValue();
        bool clearLogEnabled = cvarManager->getCvar("ChatLogger_clearLog").getBoolValue();

        if (saveChatsAndClearLogEnabled) {
            saveChatsAndClearLog();
        }
        else if (clearLogEnabled) {
            clearLog("ChatLog.json");
        }
        });

    LOG("Chat Logger is loaded!");
}

void ChatLogger::onChat(void* params)
{
    if (!params) return;
    ChatMessageParams* chatParams = reinterpret_cast<ChatMessageParams*>(params);
    PriWrapper pri(reinterpret_cast<uintptr_t>(chatParams->InPRI));
    std::string message = (UnrealStringWrapper(reinterpret_cast<uintptr_t>(&chatParams->Message))).ToString();

    if (!pri) {
        LOG("pri is NULL!");
        return;
    }
    
    // parse quickchat if necessary
    std::string quickChat = "";
    if (message.size() <= 15 && message.contains("Group") && message.contains("Message")) {
        std::vector<int> messageDigits = {};
        for (char ch : message) {
            if (isdigit(ch)) {
                int digitInt = ch - '0';
                messageDigits.push_back(digitInt);
            }
        }
        if (messageDigits.size() == 2) {
            quickChat = QuickChatGroups[messageDigits[0] - 1][messageDigits[1] - 1];
        }
        else if (messageDigits.size() == 3) {
            int secondIndex = (messageDigits[1] * 10) + messageDigits[2];
            quickChat = QuickChatGroups[messageDigits[0] - 1][secondIndex - 1];
        }
    }

    logChat(pri, message, quickChat);
}
