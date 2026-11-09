#include "MenuBar.h"

#include "imgui.h"

bool CMenuBar::Render(bool& AutoScroll) const
{
  bool QuitRequested = false;
  if (!ImGui::BeginMenuBar())
    return QuitRequested;

  if (ImGui::BeginMenu("Datei"))
  {
    if (ImGui::MenuItem("Beenden"))
      QuitRequested = true;
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu("Ansicht"))
  {
    ImGui::MenuItem("Automatisch scrollen", NULL, &AutoScroll);
    ImGui::EndMenu();
  }
  ImGui::EndMenuBar();
  return QuitRequested;
}
