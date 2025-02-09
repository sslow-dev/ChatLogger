#include "pch.h"
#include "ChatLogger.h"

void ChatLogger::RenderSettings()
{
	// lil helpers
	static auto spacing = [](int amt)
	{
		for (int i = 0; i < amt; ++i)
		{
			ImGui::Spacing();
		}
	};

	static const ImVec4 yellow = { 1, 1, 0, 1 };


	ImGui::Text(plugin_version_no_build);

	spacing(4);

	const std::string saved_chats_filename =	saved_chats_json_path.filename().string();
	const std::string chat_log_filename =		chat_log_json_path.filename().string();

	// get CVars for logging options
	auto save_quickchats_cvar =					cvarManager->getCvar(Cvars::save_quickchats);
	auto save_user_chats_cvar =					cvarManager->getCvar(Cvars::save_user_chats);
	auto save_party_chats_cvar =				cvarManager->getCvar(Cvars::save_party_chats);
	auto save_teammate_chats_cvar =				cvarManager->getCvar(Cvars::save_teammate_chats);
	auto save_opponent_chats_cvar =				cvarManager->getCvar(Cvars::save_opponent_chats);
	auto save_spectator_chats_cvar =			cvarManager->getCvar(Cvars::save_spectator_chats);
	auto save_spectated_player_chats_cvar =		cvarManager->getCvar(Cvars::save_spectated_player_chats);

	auto save_chats_and_clear_log_cvar =		cvarManager->getCvar(Cvars::save_chats_and_clear_log);
	auto clear_log_cvar =						cvarManager->getCvar(Cvars::clear_log);

	auto minify_saved_chats_cvar =				cvarManager->getCvar(Cvars::minify_saved_chats);
	auto minify_chat_log_cvar =					cvarManager->getCvar(Cvars::minify_chat_log);

	if (!save_quickchats_cvar) return;


	// Checkboxes for logging options
	ImGui::TextColored(yellow, "Choose which chats will be logged");
	ImGui::Spacing();

	bool save_quickchats = save_quickchats_cvar.getBoolValue();
	if (ImGui::Checkbox("Quick chats", &save_quickchats)) {
		save_quickchats_cvar.setValue(save_quickchats);
	}

	bool save_user_chats = save_user_chats_cvar.getBoolValue();
	if (ImGui::Checkbox("User chats (you)", &save_user_chats)) {
		save_user_chats_cvar.setValue(save_user_chats);
	}

	bool save_party_chats = save_party_chats_cvar.getBoolValue();
	if (ImGui::Checkbox("Party chats", &save_party_chats)) {
		save_party_chats_cvar.setValue(save_party_chats);
	}

	bool save_teammate_chats = save_teammate_chats_cvar.getBoolValue();
	if (ImGui::Checkbox("Teammate chats", &save_teammate_chats)) {
		save_teammate_chats_cvar.setValue(save_teammate_chats);
	}

	bool save_opponent_chats = save_opponent_chats_cvar.getBoolValue();
	if (ImGui::Checkbox("Opponent chats", &save_opponent_chats)) {
		save_opponent_chats_cvar.setValue(save_opponent_chats);
	}

	bool save_spectator_chats = save_spectator_chats_cvar.getBoolValue();
	if (ImGui::Checkbox("Spectator chats", &save_spectator_chats)) {
		save_spectator_chats_cvar.setValue(save_spectator_chats);
	}

	bool save_spectated_player_chats = save_spectated_player_chats_cvar.getBoolValue();
	if (ImGui::Checkbox("Spectated player chats", &save_spectated_player_chats)) {
		save_spectated_player_chats_cvar.setValue(save_spectated_player_chats);
	}


	// Radio buttons for handling chats
	ImGui::NewLine();
	ImGui::TextColored(yellow, "Choose what happens to match chats after each game\t(toggle saving chats)");
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("Useful toggle if you happen to change your mind about saving chats mid-game");
	}

	ImGui::Spacing();

	static int e = 0;

	if (ImGui::RadioButton("Save chats", &e, 0))
	{
		save_chats_and_clear_log_cvar.setValue(true);
		clear_log_cvar.setValue(false);
		LOG("Chats will be saved to '{}' and '{}' will be cleared after each match", saved_chats_filename, chat_log_filename);
	}

	if (ImGui::RadioButton("Clear chats", &e, 1))
	{
		save_chats_and_clear_log_cvar.setValue(false);
		clear_log_cvar.setValue(true);
		LOG("'{}' will be cleared after each match", chat_log_filename);
	}
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("Chats won't be saved!");
	}


	// Checkboxes for minifying JSON files
	ImGui::NewLine();
	ImGui::TextColored(yellow, "Choose which JSON files to minify\t(saves space at the cost of readability)");
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("Good option if JSON files will be processed programmatically");
	}

	ImGui::Spacing();

	bool minifyChatLogEnabled = minify_chat_log_cvar.getBoolValue();
	if (ImGui::Checkbox(chat_log_filename.c_str(), &minifyChatLogEnabled)) {
		minify_chat_log_cvar.setValue(minifyChatLogEnabled);
	}

	bool minifySavedChatsEnabled = minify_saved_chats_cvar.getBoolValue();
	if (ImGui::Checkbox(saved_chats_filename.c_str(), &minifySavedChatsEnabled)) {
		minify_saved_chats_cvar.setValue(minifySavedChatsEnabled);
	}


	// Buttons to clear JSON files
	spacing(10);
	ImGui::TextColored(yellow, "Clear all chats in JSON file");
	ImGui::Spacing();

	ImGui::PushID(0);
	ImGui::PushStyleColor(ImGuiCol_Button,			ImColor::HSV(0.0f, 0.6f, 0.6f).Value);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered,	ImColor::HSV(0.0f, 0.7f, 0.7f).Value);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive,	ImColor::HSV(0.0f, 0.8f, 0.8f).Value);

	std::string label = "Clear " + chat_log_filename;
	if (ImGui::Button(label.c_str())) {
		cvarManager->executeCommand(Commands::clear_chat_log);
	}
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("Erase all chats in '%s'\t** PERMANENT **", chat_log_filename);
	}

	ImGui::Spacing();

	std::string clearSavedChatsButton = "Clear " + saved_chats_filename;
	if (ImGui::Button(clearSavedChatsButton.c_str()))
	{
		cvarManager->executeCommand(Commands::clear_saved_chats);
	}
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip(("Erase all chats in '" + saved_chats_filename + "'\t** PERMANENT **").c_str());
	}

	ImGui::PopStyleColor(3);
	ImGui::PopID();
}