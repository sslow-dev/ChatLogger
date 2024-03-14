#include "pch.h"
#include "ChatLogger.h"

void ChatLogger::RenderSettings() {
    // get CVars for logging options
    CVarWrapper quickChatsCvar = cvarManager->getCvar("ChatLogger_quickChatsEnabled");
    CVarWrapper userChatsCvar = cvarManager->getCvar("ChatLogger_userChatsEnabled");
    CVarWrapper teammateChatsCvar = cvarManager->getCvar("ChatLogger_teammateChatsEnabled");
    CVarWrapper opponentChatsCvar = cvarManager->getCvar("ChatLogger_opponentChatsEnabled");
    CVarWrapper spectatorChatsCvar = cvarManager->getCvar("ChatLogger_spectatorChatsEnabled");
    CVarWrapper spectatedPlayerChatsCvar = cvarManager->getCvar("ChatLogger_spectatedPlayerChatsEnabled");
    CVarWrapper partyChatsCvar = cvarManager->getCvar("ChatLogger_partyChatsEnabled");

    // get CVars for handling 'ChatLog.json'
    CVarWrapper saveChatsAndClearLogCvar = cvarManager->getCvar("ChatLogger_saveChatsAndClearLog");
    CVarWrapper clearLogCvar = cvarManager->getCvar("ChatLogger_clearLog");

    // get CVars for minify options
    CVarWrapper minifySavedChatsCvar = cvarManager->getCvar("ChatLogger_minifySavedChats");
    CVarWrapper minifyChatLogCvar = cvarManager->getCvar("ChatLogger_minifyChatLog");

    std::string savedChatsFileName = currentMonth + "_SavedChats.json";

    // Checkboxes for logging options
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Choose which chats will be logged");
    ImGui::Spacing();

    if (!quickChatsCvar) { return; }
    bool quickChatsEnabled = quickChatsCvar.getBoolValue();
    if (ImGui::Checkbox("Quick chats", &quickChatsEnabled)) {
        quickChatsCvar.setValue(quickChatsEnabled);
    }
    if (!userChatsCvar) { return; }
    bool userChatsEnabled = userChatsCvar.getBoolValue();
    if (ImGui::Checkbox("User chats (you)", &userChatsEnabled)) {
        userChatsCvar.setValue(userChatsEnabled);
    }
    if (!partyChatsCvar) { return; }
    bool partyChatsEnabled = partyChatsCvar.getBoolValue();
    if (ImGui::Checkbox("Party chats", &partyChatsEnabled)) {
        partyChatsCvar.setValue(partyChatsEnabled);
    }
    if (!teammateChatsCvar) { return; }
    bool teammateChatsEnabled = teammateChatsCvar.getBoolValue();
    if (ImGui::Checkbox("Teammate chats", &teammateChatsEnabled)) {
        teammateChatsCvar.setValue(teammateChatsEnabled);
    }
    if (!opponentChatsCvar) { return; }
    bool opponentChatsEnabled = opponentChatsCvar.getBoolValue();
    if (ImGui::Checkbox("Opponent chats", &opponentChatsEnabled)) {
        opponentChatsCvar.setValue(opponentChatsEnabled);
    }
    if (!spectatorChatsCvar) { return; }
    bool spectatorChatsEnabled = spectatorChatsCvar.getBoolValue();
    if (ImGui::Checkbox("Spectator chats", &spectatorChatsEnabled)) {
        spectatorChatsCvar.setValue(spectatorChatsEnabled);
    }
    if (!spectatedPlayerChatsCvar) { return; }
    bool spectatedPlayerChatsEnabled = spectatedPlayerChatsCvar.getBoolValue();
    if (ImGui::Checkbox("Spectated player chats", &spectatedPlayerChatsEnabled)) {
        spectatedPlayerChatsCvar.setValue(spectatedPlayerChatsEnabled);
    }

    // Radio buttons for handling chats
    ImGui::NewLine();
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Choose what happens to match chats after each game\t(toggle saving chats)");
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Useful toggle if you happen to change your mind about saving chats mid-game");
    }
    ImGui::Spacing();

    static int e = 0;
    if (!saveChatsAndClearLogCvar) { return; }
    if (!clearLogCvar) { return; }
    if (ImGui::RadioButton("Save chats", &e, 0)) {
        saveChatsAndClearLogCvar.setValue(true);
        clearLogCvar.setValue(false);
        LOG("Chats will be saved to '{}' and 'ChatLog.json' will be cleared after each match", savedChatsFileName);
    }
    if (ImGui::RadioButton("Clear chats", &e, 1)) {
        saveChatsAndClearLogCvar.setValue(false);
        clearLogCvar.setValue(true);
        LOG("'ChatLog.json' will be cleared after each match");
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Chats won't be saved!");
    }

    // Checkboxes for minifying JSON files
    ImGui::NewLine();
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Choose which JSON files to minify\t(saves space at the cost of readability)");
    ImGui::Spacing();
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Good option if JSON files will be processed programmatically");
    }

    if (!minifyChatLogCvar) { return; }
    bool minifyChatLogEnabled = minifyChatLogCvar.getBoolValue();
    if (ImGui::Checkbox("ChatLog.json", &minifyChatLogEnabled)) {
        minifyChatLogCvar.setValue(minifyChatLogEnabled);
    }
    if (!minifySavedChatsCvar) { return; }
    bool minifySavedChatsEnabled = minifySavedChatsCvar.getBoolValue();
    if (ImGui::Checkbox(savedChatsFileName.c_str(), &minifySavedChatsEnabled)) {
        minifySavedChatsCvar.setValue(minifySavedChatsEnabled);
    }

    // Buttons to clear JSON files
    ImGui::NewLine();
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Clear all chats in JSON file");
    ImGui::Spacing();

    ImGui::PushID(0);
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));
    if (ImGui::Button("Clear ChatLog.json")) {
        cvarManager->executeCommand("ChatLogger_clearChatLog");
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Erase all chats in 'ChatLog.json'\t** PERMANENT **");
    }
    ImGui::NewLine();
    std::string clearSavedChatsButton = "Clear " + savedChatsFileName;
    if (ImGui::Button(clearSavedChatsButton.c_str(), ImVec2(0, 0))) {
        cvarManager->executeCommand("ChatLogger_clearSavedChats");
    }
    ImGui::PopStyleColor(3);
    ImGui::PopID();
    std::string clearSavedChatsToolTipStr = "Erase all chats in '" + savedChatsFileName + "'\t** PERMANENT **";
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(clearSavedChatsToolTipStr.c_str());
    }
}