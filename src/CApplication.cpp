#include "CApplication.h"
#include "CScpiResponse.h"

#include "implot.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#include <cstdlib>
#include <cstring>
#include <windows.h>

CApplication::CApplication()
  : m_Host("127.0.0.1"),
    m_Port("23"),
    m_SerialDevice("COM1"),
    m_BaudRate(9600),
    m_Transport(0),
    m_Connected(false),
    m_AutoScroll(true),
    m_Status("Nicht verbunden")
{
  m_Command[0] = '\0';
  m_Communication.Start();
}

CApplication::~CApplication()
{
  m_Communication.Stop();
}

void CApplication::Render()
{
  ProcessEvents();

  ImGuiViewport* Viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(Viewport->WorkPos);
  ImGui::SetNextWindowSize(Viewport->WorkSize);
  ImGui::Begin("DevTerm", NULL,
               ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
               ImGuiWindowFlags_MenuBar);

  RenderMenu();
  RenderToolbar();
  RenderConnectionSettings();
  ImGui::Separator();
  RenderTabs();
  RenderCommandInput();
  RenderStatusBar();
  ImGui::End();
}

void CApplication::RenderMenu()
{
  if (!ImGui::BeginMenuBar())
    return;

  if (ImGui::BeginMenu("Datei"))
  {
    if (ImGui::MenuItem("Beenden"))
      PostQuitMessage(0);
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu("Ansicht"))
  {
    ImGui::MenuItem("Automatisch scrollen", NULL, &m_AutoScroll);
    ImGui::EndMenu();
  }
  ImGui::EndMenuBar();
}

void CApplication::RenderToolbar()
{
  if (m_Connected)
  {
    if (ImGui::Button("Trennen"))
    {
      m_Communication.Disconnect();
      m_Status = "Trenne Verbindung ...";
    }
  }
  else if (ImGui::Button("Verbinden"))
  {
    if (m_Transport == 0)
    {
      int Port = std::atoi(m_Port.c_str());
      if (m_Host.empty() || Port < 1 || Port > 65535)
      {
        m_Status = "Bitte einen Host und gueltigen TCP-Port eingeben.";
        AddLogLine(m_Status);
      }
      else
      {
        m_Communication.ConnectTcp(m_Host, Port);
        m_Status = "Verbinde ...";
      }
    }
    else
    {
      if (m_SerialDevice.empty() || m_BaudRate <= 0)
      {
        m_Status = "Bitte einen seriellen Port und eine Baudrate eingeben.";
        AddLogLine(m_Status);
      }
      else
      {
        m_Communication.ConnectSerial(m_SerialDevice, m_BaudRate);
        m_Status = "Verbinde ...";
      }
    }
  }

  ImGui::SameLine();
  if (ImGui::Button("Protokoll leeren"))
    m_Log.clear();
}

void CApplication::RenderConnectionSettings()
{
  ImGui::SameLine();
  ImGui::SetNextItemWidth(125.0f);
  const char* Transports[] = {"TCP", "Seriell"};
  ImGui::BeginDisabled(m_Connected);
  ImGui::Combo("Verbindung", &m_Transport, Transports, 2);
  ImGui::SameLine();
  if (m_Transport == 0)
  {
    ImGui::SetNextItemWidth(170.0f);
    ImGui::InputText("Host", &m_Host);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(75.0f);
    ImGui::InputText("Port", &m_Port);
  }
  else
  {
    ImGui::SetNextItemWidth(120.0f);
    ImGui::InputText("Port (COM)", &m_SerialDevice);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100.0f);
    ImGui::InputInt("Baud", &m_BaudRate);
  }
  ImGui::EndDisabled();
}

void CApplication::RenderTabs()
{
  if (!ImGui::BeginTabBar("Arbeitsbereich"))
    return;

  if (ImGui::BeginTabItem("Protokoll"))
  {
    ImGui::BeginChild("Protokollinhalt", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);
    for (size_t Index = 0; Index < m_Log.size(); ++Index)
      ImGui::TextUnformatted(m_Log[Index].c_str());
    if (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
      ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();
    ImGui::EndTabItem();
  }

  if (ImGui::BeginTabItem("Plot"))
  {
    if (m_PlotY.empty())
      ImGui::TextDisabled("Numerische Einzelwerte aus Geraeteantworten erscheinen hier.");
    else if (ImPlot::BeginPlot("Messwerte", ImVec2(-1, -1)))
    {
      ImPlot::SetupAxes("Messung", "Wert");
      ImPlot::PlotLine("Antwort", &m_PlotX[0], &m_PlotY[0],
                       static_cast<int>(m_PlotY.size()));
      ImPlot::EndPlot();
    }
    ImGui::EndTabItem();
  }

  ImGui::EndTabBar();
}

void CApplication::RenderCommandInput()
{
  ImGui::Separator();
  ImGui::SetNextItemWidth(-90.0f);
  bool Submit = ImGui::InputText("##Befehl", m_Command, sizeof(m_Command),
                                 ImGuiInputTextFlags_EnterReturnsTrue);
  ImGui::SameLine();
  if (ImGui::Button("Senden"))
    Submit = true;
  if (Submit)
    SendCommand();
}

void CApplication::RenderStatusBar()
{
  ImGui::Separator();
  ImGui::Text("%s", m_Connected ? "Verbunden" : m_Status.c_str());
  ImGui::SameLine(ImGui::GetWindowWidth() - 165.0f);
  ImGui::TextDisabled("DevTerm 0.1.0");
}

void CApplication::ProcessEvents()
{
  std::vector<CCommunicationEvent> Events;
  m_Communication.TakeEvents(Events);
  for (size_t Index = 0; Index < Events.size(); ++Index)
  {
    const CCommunicationEvent& Event = Events[Index];
    if (Event.Type == CommunicationEventConnected)
    {
      m_Connected = true;
      m_Status = "Verbunden";
    }
    else if (Event.Type == CommunicationEventDisconnected)
    {
      m_Connected = false;
      if (m_Status.compare(0, 7, "Fehler:") != 0)
        m_Status = "Nicht verbunden";
    }
    else if (Event.Type == CommunicationEventError)
    {
      m_Status = "Fehler: " + Event.Text;
      AddLogLine("! " + Event.Text);
    }
    else if (Event.Type == CommunicationEventLog)
      AddLogLine(Event.Text);
    else if (Event.Type == CommunicationEventResponse)
    {
      AddLogLine("< " + Event.Text);
      double Value;
      if (ParsePlotValue(Event.Text, Value))
      {
        m_PlotX.push_back(static_cast<double>(m_PlotX.size() + 1));
        m_PlotY.push_back(Value);
        if (m_PlotY.size() > 1000)
        {
          m_PlotX.erase(m_PlotX.begin());
          m_PlotY.erase(m_PlotY.begin());
          for (size_t Point = 0; Point < m_PlotX.size(); ++Point)
            m_PlotX[Point] = static_cast<double>(Point + 1);
        }
      }
    }
  }
}

void CApplication::AddLogLine(const std::string& Line)
{
  m_Log.push_back(Line);
  if (m_Log.size() > 5000)
    m_Log.erase(m_Log.begin(), m_Log.begin() + 1000);
}

void CApplication::SendCommand()
{
  if (m_Command[0] == '\0')
    return;
  m_Communication.Send(m_Command);
  m_Command[0] = '\0';
}
