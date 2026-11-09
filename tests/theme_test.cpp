#include "theme/spectrum.h"
#include "CCommunicationWorker.h"
#include "CScpiResponse.h"
#include "CWindowsFont.h"
#include "imgui.h"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>

static bool Check(bool Condition, const char* Message)
{
  if (!Condition)
    std::cerr << Message << std::endl;
  return Condition;
}

static bool TestTheme()
{
  ImGui::CreateContext();
  ImGui::Spectrum::StyleColorsSpectrum(true);
  ImGuiStyle& Style = ImGui::GetStyle();
  const ImVec4 DarkBackground = Style.Colors[ImGuiCol_WindowBg];
  const ImU32 DarkFrame = ImGui::ColorConvertFloat4ToU32(Style.Colors[ImGuiCol_FrameBg]);
  const ImU32 DarkButton = ImGui::ColorConvertFloat4ToU32(Style.Colors[ImGuiCol_Button]);
  const ImGui::Spectrum::SpectrumPalette* DarkPalette = ImGui::Spectrum::Colors;
  bool Passed = Check(DarkBackground.x < 0.2f, "Dark Spectrum background is not dark.");
  Passed = Check(Style.Colors[ImGuiCol_TabSelectedOverline].z >
                 Style.Colors[ImGuiCol_TabSelectedOverline].x,
                 "Selected tab must use the blue Spectrum accent.") && Passed;

  ImGui::Spectrum::StyleColorsSpectrum(false);
  Passed = Check(ImGui::Spectrum::Colors != DarkPalette, "Palette did not change.") && Passed;
  const ImGuiCol PanelColors[] = { ImGuiCol_WindowBg, ImGuiCol_ChildBg, ImGuiCol_PopupBg,
                                   ImGuiCol_MenuBarBg, ImGuiCol_Button };
  for (size_t Index = 0; Index < sizeof(PanelColors) / sizeof(PanelColors[0]); ++Index)
  {
    Passed = Check(ImGui::ColorConvertFloat4ToU32(Style.Colors[PanelColors[Index]]) ==
                   IM_COL32(0xD6, 0xD6, 0xD6, 0xFF),
                   "Light panel and button backgrounds must be #D6D6D6.") && Passed;
  }
  const ImGuiCol InputColors[] = { ImGuiCol_FrameBg, ImGuiCol_FrameBgHovered,
                                   ImGuiCol_FrameBgActive };
  for (size_t Index = 0; Index < sizeof(InputColors) / sizeof(InputColors[0]); ++Index)
  {
    Passed = Check(ImGui::ColorConvertFloat4ToU32(Style.Colors[InputColors[Index]]) ==
                   IM_COL32(255, 255, 255, 255),
                   "Light input backgrounds must remain white in every state.") && Passed;
  }
  Passed = Check(Style.Colors[ImGuiCol_CheckboxSelectedBg].z >
                 Style.Colors[ImGuiCol_CheckboxSelectedBg].x,
                 "Selected checkbox must use the blue Spectrum accent.") && Passed;

  ImGui::Spectrum::StyleColorsSpectrum(true);
  Passed = Check(Style.Colors[ImGuiCol_WindowBg].x == DarkBackground.x,
                 "Switching back to dark did not restore the palette.") && Passed;
  Passed = Check(ImGui::ColorConvertFloat4ToU32(Style.Colors[ImGuiCol_FrameBg]) == DarkFrame &&
                 ImGui::ColorConvertFloat4ToU32(Style.Colors[ImGuiCol_Button]) == DarkButton,
                 "Dark controls must remain unchanged.") && Passed;
  Passed = Check(!LoadWindowsFont(L"Z:\\DevTerm-nonexistent-font.ttf", 13.0f),
                 "A missing font must report an error.") && Passed;
  Passed = Check(LoadWindowsDefaultFont(13.0f), "Segoe UI was not loaded.") && Passed;
  Passed = Check(ImGui::GetIO().FontDefault != NULL, "Default font was not set.") && Passed;
  ImGui::DestroyContext();
  return Passed;
}

static bool WaitForEvent(CCommunicationWorker& Worker, ECommunicationEventType Type,
                         const std::string& Text)
{
  for (int i = 0; i < 100; ++i)
  {
    std::vector<CCommunicationEvent> Events;
    Worker.TakeEvents(Events);
    for (size_t Index = 0; Index < Events.size(); ++Index)
    {
      if (Events[Index].Type == CommunicationEventError)
      {
        std::cerr << Events[Index].Text << std::endl;
        return false;
      }
      if (Events[Index].Type == Type && (Text.empty() || Events[Index].Text == Text))
        return true;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
  std::cerr << "Timed out waiting for communication event." << std::endl;
  return false;
}

static bool TestDemoServer(int Port)
{
  CCommunicationWorker Worker;
  Worker.Start();
  Worker.ConnectTcp("127.0.0.1", Port);
  if (!WaitForEvent(Worker, CommunicationEventConnected, ""))
    return false;
  Worker.Send("*VER ?");
  if (!WaitForEvent(Worker, CommunicationEventResponse, ":VER 10.234"))
    return false;
  Worker.Send("*MEAS:VOLT ?");
  bool MeasurementReceived = false;
  for (int i = 0; i < 100 && !MeasurementReceived; ++i)
  {
    std::vector<CCommunicationEvent> Events;
    Worker.TakeEvents(Events);
    for (size_t Index = 0; Index < Events.size(); ++Index)
    {
      if (Events[Index].Type == CommunicationEventError)
      {
        std::cerr << Events[Index].Text << std::endl;
        return false;
      }
      if (Events[Index].Type == CommunicationEventResponse &&
          Events[Index].Text.compare(0, 11, ":MEAS:VOLT ") == 0)
      {
        double Value;
        if (!ParsePlotValue(Events[Index].Text, Value) || Value < 10.0 || Value > 14.0)
          return Check(false, "Invalid voltage measurement received.");
        MeasurementReceived = true;
      }
    }
    if (!MeasurementReceived)
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
  if (!Check(MeasurementReceived, "Timed out waiting for voltage measurement."))
    return false;
  Worker.Disconnect();
  return WaitForEvent(Worker, CommunicationEventDisconnected, "");
}

int main(int ArgumentCount, char** Arguments)
{
  if (!TestTheme())
    return 1;
  double Value = 0;
  if (!Check(ParsePlotValue(":MEAS:VOLT 12.345", Value) && Value == 12.345,
             "SCPI measurement was not parsed.") ||
      !Check(ParsePlotValue(" 1.5 ", Value) && Value == 1.5,
             "Legacy numeric response was not parsed.") ||
      !Check(!ParsePlotValue("*MEAS:VOLT ?", Value), "Echo must not be plotted.") ||
      !Check(!ParsePlotValue(":VER 10.234", Value), "Version must not be plotted.") ||
      !Check(!ParsePlotValue(":MEAS:VOLT nan", Value), "Nonfinite value must not be plotted.") ||
      !Check(!ParsePlotValue(":MEAS:VOLT 1 invalid", Value), "Invalid value must not be plotted."))
    return 1;
  if (ArgumentCount == 2 && !TestDemoServer(std::atoi(Arguments[1])))
    return 1;
  return 0;
}
