#include "StatusBar.h"

#include "imgui.h"

void CStatusBar::Render(bool Connected, const std::string& Status) const
{
  ImGui::Separator();
  ImGui::Text("%s", Connected ? "Verbunden" : Status.c_str());
  ImGui::SameLine(ImGui::GetWindowWidth() - 165.0f);
  ImGui::TextDisabled("DevTerm 0.1.0");
}
