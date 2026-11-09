#include "CommandInput.h"

#include "imgui.h"

bool CCommandInput::Render(char* Command, std::size_t CommandSize) const
{
  ImGui::Separator();
  ImGui::SetNextItemWidth(-90.0f);
  bool Submit = ImGui::InputText("##Befehl", Command, CommandSize,
                                 ImGuiInputTextFlags_EnterReturnsTrue);
  ImGui::SameLine();
  if (ImGui::Button("Senden"))
    Submit = true;
  return Submit;
}
