#include "Application.h"
#include "ScpiResponse.h"

#include "imgui.h"

#include <cstdlib>
#include <windows.h>

CApplication::CApplication()
  : m_ConnectionSettings(),
    m_Connected(false),
    m_AutoScroll(true),
    m_Status("Nicht verbunden")
{
  m_ConnectionSettings.Host = "127.0.0.1";
  m_ConnectionSettings.Port = "23";
  m_ConnectionSettings.SerialDevice = "COM1";
  m_ConnectionSettings.BaudRate = 9600;
  m_ConnectionSettings.Transport = 0;
  m_Command[0] = '\0';
  m_Communication.Start();
}

CApplication::~CApplication()
{
  m_Communication.Stop();
}

void CApplication::Render(HWND Window, bool Active, bool TransparentBackground)
{
  ProcessEvents();

  ImGuiViewport* Viewport = ImGui::GetMainViewport();
  m_TitleBar.Render(Window, Active, TransparentBackground);

  ImGui::SetNextWindowPos(ImVec2(Viewport->Pos.x,
                                 Viewport->Pos.y + CTitleBar::HEIGHT));
  ImGui::SetNextWindowSize(ImVec2(
      Viewport->Size.x, Viewport->Size.y - CTitleBar::HEIGHT));
  ImGuiWindowFlags WindowFlags =
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
      ImGuiWindowFlags_MenuBar;
  ImGui::Begin("DevTerm", NULL, WindowFlags);

  if (m_MenuBar.Render(m_AutoScroll))
    PostQuitMessage(0);
  HandleConnectionAction(m_ConnectionBar.Render(m_ConnectionSettings,
                                                m_Connected));
  ImGui::Separator();
  m_WorkspaceTabs.Render(m_Log, m_PlotX, m_PlotY, m_AutoScroll);
  if (m_CommandInput.Render(m_Command, sizeof(m_Command)))
    SendCommand();
  m_StatusBar.Render(m_Connected, m_Status);
  ImGui::End();
}

void CApplication::HandleConnectionAction(CConnectionAction Action)
{
  if (Action == ConnectionActionDisconnect)
  {
    m_Communication.Disconnect();
    m_Status = "Trenne Verbindung ...";
  }
  else if (Action == ConnectionActionClearLog)
  {
    m_Log.clear();
  }
  else if (Action == ConnectionActionConnect)
  {
    if (m_ConnectionSettings.Transport == 0)
    {
      int Port = std::atoi(m_ConnectionSettings.Port.c_str());
      if (m_ConnectionSettings.Host.empty() || Port < 1 || Port > 65535)
      {
        m_Status = "Bitte einen Host und gueltigen TCP-Port eingeben.";
        AddLogLine(m_Status);
      }
      else
      {
        m_Communication.ConnectTcp(m_ConnectionSettings.Host, Port);
        m_Status = "Verbinde ...";
      }
    }
    else
    {
      if (m_ConnectionSettings.SerialDevice.empty() ||
          m_ConnectionSettings.BaudRate <= 0)
      {
        m_Status = "Bitte einen seriellen Port und eine Baudrate eingeben.";
        AddLogLine(m_Status);
      }
      else
      {
        m_Communication.ConnectSerial(m_ConnectionSettings.SerialDevice,
                                      m_ConnectionSettings.BaudRate);
        m_Status = "Verbinde ...";
      }
    }
  }
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
