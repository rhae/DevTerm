#include "TitleBar.h"

#include "imgui.h"

namespace
{
const float ButtonWidth = 46.0f;
const float IconSize = 24.0f;

void DrawApplicationIcon(ImDrawList* DrawList, const ImVec2& Position)
{
  const ImVec2 Max(Position.x + IconSize, Position.y + IconSize);
  DrawList->AddRectFilled(Position, Max, IM_COL32(38, 128, 235, 255), 5.0f);

  const ImU32 IconColor = IM_COL32(255, 255, 255, 255);
  DrawList->AddLine(ImVec2(Position.x + 7.0f, Position.y + 8.0f),
                    ImVec2(Position.x + 11.0f, Position.y + 12.0f),
                    IconColor, 2.0f);
  DrawList->AddLine(ImVec2(Position.x + 11.0f, Position.y + 12.0f),
                    ImVec2(Position.x + 7.0f, Position.y + 16.0f),
                    IconColor, 2.0f);
  DrawList->AddLine(ImVec2(Position.x + 13.0f, Position.y + 16.0f),
                    ImVec2(Position.x + 18.0f, Position.y + 16.0f),
                    IconColor, 2.0f);
}

bool DrawTitleBarButton(const char* Id, HWND Window, int Button,
                       bool Maximized, float Width)
{
  const float ButtonHeight = static_cast<float>(CTitleBar::HEIGHT);
  ImGui::InvisibleButton(Id, ImVec2(Width, ButtonHeight));

  const bool Hovered = ImGui::IsItemHovered();
  const ImVec2 Min = ImGui::GetItemRectMin();
  const ImVec2 Max = ImGui::GetItemRectMax();
  ImDrawList* DrawList = ImGui::GetWindowDrawList();
  if (Hovered)
  {
    const ImU32 HoverColor = Button == 2
        ? IM_COL32(196, 43, 48, 255)
        : IM_COL32(72, 79, 88, 255);
    DrawList->AddRectFilled(Min, Max, HoverColor);
  }

  const float CenterX = (Min.x + Max.x) * 0.5f;
  const float CenterY = (Min.y + Max.y) * 0.5f;
  const ImU32 IconColor = IM_COL32(255, 255, 255, 255);
  if (Button == 0)
  {
    DrawList->AddLine(ImVec2(CenterX - 5.0f, CenterY + 3.0f),
                      ImVec2(CenterX + 5.0f, CenterY + 3.0f),
                      IconColor, 1.5f);
  }
  else if (Button == 1 && Maximized)
  {
    DrawList->AddRect(ImVec2(CenterX - 4.0f, CenterY - 5.0f),
                      ImVec2(CenterX + 5.0f, CenterY + 3.0f),
                      IconColor, 0.0f, 0, 1.5f);
    DrawList->AddRect(ImVec2(CenterX - 6.0f, CenterY - 3.0f),
                      ImVec2(CenterX + 3.0f, CenterY + 5.0f),
                      IconColor, 0.0f, 0, 1.5f);
  }
  else if (Button == 1)
  {
    DrawList->AddRect(ImVec2(CenterX - 5.0f, CenterY - 5.0f),
                      ImVec2(CenterX + 5.0f, CenterY + 5.0f),
                      IconColor, 0.0f, 0, 1.5f);
  }
  else
  {
    DrawList->AddLine(ImVec2(CenterX - 5.0f, CenterY - 5.0f),
                      ImVec2(CenterX + 5.0f, CenterY + 5.0f),
                      IconColor, 1.5f);
    DrawList->AddLine(ImVec2(CenterX + 5.0f, CenterY - 5.0f),
                      ImVec2(CenterX - 5.0f, CenterY + 5.0f),
                      IconColor, 1.5f);
  }

  if (!ImGui::IsItemClicked())
    return false;

  if (Button == 0)
    ShowWindow(Window, SW_MINIMIZE);
  else if (Button == 1)
    ShowWindow(Window, Maximized ? SW_RESTORE : SW_MAXIMIZE);
  else
    PostMessageW(Window, WM_CLOSE, 0, 0);
  return true;
}
}

void CTitleBar::Render(HWND Window, bool Active, bool AcrylicEnabled) const
{
  ImGuiViewport* Viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(Viewport->Pos);
  ImGui::SetNextWindowSize(ImVec2(Viewport->Size.x,
                                  static_cast<float>(HEIGHT)));
  ImGui::SetNextWindowViewport(Viewport->ID);

  const ImVec4 Background = Active
      ? ImVec4(0.10f, 0.12f, 0.15f, AcrylicEnabled ? 0.78f : 1.0f)
      : ImVec4(0.15f, 0.16f, 0.18f, AcrylicEnabled ? 0.82f : 1.0f);
  ImGui::PushStyleColor(ImGuiCol_WindowBg, Background);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 0.0f));

  const ImGuiWindowFlags Flags =
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
      ImGuiWindowFlags_NoBringToFrontOnFocus |
      ImGuiWindowFlags_NoNavFocus;
  if (ImGui::Begin("##DevTermTitleBar", NULL, Flags))
  {
    const float CenterY = (static_cast<float>(HEIGHT) - IconSize) * 0.5f;
    const ImVec2 IconPosition = ImGui::GetCursorScreenPos();
    DrawApplicationIcon(ImGui::GetWindowDrawList(),
                        ImVec2(IconPosition.x, IconPosition.y + CenterY));

    ImGui::SetCursorPos(ImVec2(10.0f + IconSize + 10.0f,
                               (static_cast<float>(HEIGHT) -
                                ImGui::GetTextLineHeight()) * 0.5f));
    ImGui::TextUnformatted("DevTerm");
    ImGui::SameLine(0.0f, 8.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.78f, 0.81f, 0.85f, 1.0f));
    ImGui::TextUnformatted("SCPI Terminal");
    ImGui::PopStyleColor();

    const float ControlsWidth = ButtonWidth * 3.0f;
    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - ControlsWidth, 0.0f));
    const bool Maximized = IsZoomed(Window) != FALSE;
    DrawTitleBarButton("##Minimize", Window, 0, Maximized, ButtonWidth);
    ImGui::SameLine(0.0f, 0.0f);
    DrawTitleBarButton("##Maximize", Window, 1, Maximized, ButtonWidth);
    ImGui::SameLine(0.0f, 0.0f);
    DrawTitleBarButton("##Close", Window, 2, Maximized, ButtonWidth);
    ImGui::End();
  }

  ImGui::PopStyleVar(3);
  ImGui::PopStyleColor();
}
