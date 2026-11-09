#include "ConnectionBar.h"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

CConnectionAction CConnectionBar::Render(CConnectionSettings& Settings,
                                         bool Connected) const
{
  CConnectionAction Action = ConnectionActionNone;
  if (Connected)
  {
    if (ImGui::Button("Trennen"))
      Action = ConnectionActionDisconnect;
  }
  else if (ImGui::Button("Verbinden"))
  {
    Action = ConnectionActionConnect;
  }

  ImGui::SameLine();
  if (ImGui::Button("Protokoll leeren"))
    Action = ConnectionActionClearLog;

  ImGui::SameLine();
  ImGui::SetNextItemWidth(125.0f);
  const char* Transports[] = {"TCP", "Seriell"};
  ImGui::BeginDisabled(Connected);
  ImGui::Combo("Verbindung", &Settings.Transport, Transports, 2);
  ImGui::SameLine();
  if (Settings.Transport == 0)
  {
    ImGui::SetNextItemWidth(170.0f);
    ImGui::InputText("Host", &Settings.Host);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(75.0f);
    ImGui::InputText("Port", &Settings.Port);
  }
  else
  {
    ImGui::SetNextItemWidth(120.0f);
    ImGui::InputText("Port (COM)", &Settings.SerialDevice);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100.0f);
    ImGui::InputInt("Baud", &Settings.BaudRate);
  }
  ImGui::EndDisabled();
  return Action;
}
