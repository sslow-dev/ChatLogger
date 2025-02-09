#pragma once
#include "Utils.hpp"


class FString
{
	const wchar_t*	ArrayData;
	int32_t			ArrayCount;
	int32_t			ArrayMax;

public:

	inline UnrealStringWrapper to_wrapper() const
	{
		return reinterpret_cast<uintptr_t>(this);
	}
};


enum class EOnlinePlatform : uint8_t
{
	OnlinePlatform_Unknown =				0,
	OnlinePlatform_Steam =					1,
	OnlinePlatform_PS4 =					2,
	OnlinePlatform_PS3 =					3,
	OnlinePlatform_Dingo =					4,
	OnlinePlatform_QQ_DEPRECATED =			5,
	OnlinePlatform_OldNNX =					6,
	OnlinePlatform_NNX =					7,
	OnlinePlatform_PsyNet =					8,
	OnlinePlatform_Deleted =				9,
	OnlinePlatform_WeGame_DEPRECATED =		10,
	OnlinePlatform_Epic =					11,
	OnlinePlatform_END =					12
};


struct UniqueNetId
{
	uint64_t				Uid;
	uint8_t					NpId[0x28];
	class FString			EpicAccountId;
	EOnlinePlatform			Platform;
	uint8_t					SplitscreenID;
};


// used by:
// Function TAGame.HUDBase_TA.AddChatMessage
// Function TAGame.GFxData_Chat_TA.AddPresetMessage
// Function TAGame.GFxData_Chat_TA.AddChatMessage
struct FGFxChatMessage
{
	int32_t					Team;
	class FString			PlayerName;
	class FString			Message;
	uint8_t					ChatChannel;
	uint32_t				bLocalPlayer : 1;
	uint32_t				bTransient : 1;
	struct UniqueNetId		SenderId;
	uint32_t				bPreset : 1;
	uint8_t					MessageType;
	class FString			TimeStamp;
};


// used by:
// Function TAGame.GFxData_Chat_TA.OnChatMessage
struct OnChatMessage_Params
{
	int32_t					Team;
	class FString			PlayerName;
	class FString			Message;
	uint8_t					ChatChannel;
	uint32_t				bLocalPlayer : 1;
	struct UniqueNetId		SenderId;
	uint8_t					MessageType;
	class FString			TimeStamp;
};


// used by:
// Function TAGame.PlayerController_TA.ChatMessage_TA
struct ChatMessage_TA_Params
{
	void*					InPRI;
	class FString			Message;
	uint8_t					ChatChannel;
	uint32_t				bPreset : 1;
	int32_t					TimeStamp;
};



// ================================== plugin specific ==================================

struct SavedChatMessage
{
	std::string name;
	std::string chat;
	std::string relation;
	std::string time;
	std::string platform;
};


// credit to https://github.com/JulienML/BetterChat/ thx fam
const std::map<std::string, std::string> quickchat_ids_to_text =
{
		{"Group1Message1", "I got it!"}, // Je l'ai !
		{"Group1Message2", "Need boost!"}, // Besoin de turbo !
		{"Group1Message3", "Take the shot!"}, // Prends-le !
		{"Group1Message4", "Defending."}, // Je défends.
		{"Group1Message5", "Go for it!"}, // Vas-y !
		{"Group1Message6", "Centering!"}, // Centre !
		{"Group1Message7", "All yours."}, // Il est pour toi.
		{"Group1Message8", "In position."}, // En position.
		{"Group1Message9", "Incoming!"}, // En approche !
		{"Group1Message10", "Faking."}, // La feinte.
		{"Group1Message11", "Bumping!"}, // Impact !
		{"Group1Message12", "On your left!"}, // Sur ta gauche !
		{"Group1Message13", "On your right!"}, // Sur ta droite !
		{"Group1Message14", "Passing!"}, // La passe !
		{"Group1Message15", "Rotating Up!"}, // Je monte !
		{"Group1Message16", "Rotating back!"}, // Je recule !
		{"Group1Message17", "You have time!"}, // Tu as le temps !

		{"Group2Message1", "Nice shot!"}, // Beau tir !
		{"Group2Message2", "Great pass!"}, // Belle passe !
		{"Group2Message3", "Thanks!"}, // Merci !
		{"Group2Message4", "What a save!"}, // Quel arrêt !
		{"Group2Message5", "Nice one!"}, // Bien vu !
		{"Group2Message6", "What a play!"}, // Quelle intensité !
		{"Group2Message7", "Great clear!"}, // Beau dégagement !
		{"Group2Message8", "Nice block!"}, // Super blocage !
		{"Group2Message9", "Nice bump!"}, // Bel impact !
		{"Group2Message10", "Nice demo!"}, // Jolie démo !
		{"Group2Message11", "We got this."}, // On assure !

		{"Group3Message1", "OMG!"}, // Oh mon dieu !
		{"Group3Message2", "Noooo!"}, // Noooon !
		{"Group3Message3", "Wow!"}, // Wow !
		{"Group3Message4", "Close one..."}, // C'était pas loin...
		{"Group3Message5", "No way!"}, // Pas possible !
		{"Group3Message6", "Holy cow!"}, // Sérieux ?!
		{"Group3Message7", "Whew."}, // Waouh.
		{"Group3Message8", "Siiiick!"}, // Truc de ouf !
		{"Group3Message9", "Calculated."}, // C'est prévu.
		{"Group3Message10", "Savage!"}, // Sauvage !
		{"Group3Message11", "Okay."}, // Ok.
		{"Group3Message12", "Yes!"}, // Oui !

		{"Group4Message1", "$#@%!"}, // $#@%!
		{"Group4Message2", "No problem."}, // Pas de problèmes.
		{"Group4Message3", "Whoops..."}, // Oups...
		{"Group4Message4", "Sorry!"}, // Désolé !
		{"Group4Message5", "My bad..."}, // Pardon...
		{"Group4Message6", "Oops!"}, // Oups !
		{"Group4Message7", "My fault."}, // Ma faute.

		{"Group5Message1", "gg"}, // gg
		{"Group5Message2", "Well played."}, // Bien joué.
		{"Group5Message3", "That was fun!"}, // C'était cool !
		{"Group5Message4", "Rematch!"}, // On remet ça !
		{"Group5Message5", "One. More. Game."}, // Encore. Une. Partie.
		{"Group5Message6", "What a game!"}, // Quelle partie !
		{"Group5Message7", "Nice moves!"}, // Super déplacements !
		{"Group5Message8", "Everybody dance!"}, // Que tout le monde dance !
		{"Group5Message9", "Party Up?"}, // On groupe ?

		{"Group6Message4", "This is Rocket League!"}, // Ça c'est Rocket League !
};
