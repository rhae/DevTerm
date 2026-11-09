#include "CConnection.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include <sstream>

CConnection::~CConnection()
{
}

CTcpConnection::CTcpConnection()
  : m_Socket(static_cast<uintptr_t>(INVALID_SOCKET))
{
}

CTcpConnection::~CTcpConnection()
{
  Close();
}

bool CTcpConnection::Open(const std::string& Endpoint, int Port, std::string& Error)
{
  Close();

  WSADATA WinsockData;
  int Result = WSAStartup(MAKEWORD(2, 2), &WinsockData);
  if (Result != 0)
  {
    Error = "Winsock konnte nicht gestartet werden (" + std::to_string(Result) + ").";
    return false;
  }

  std::ostringstream PortText;
  PortText << Port;
  addrinfo Hints = {};
  Hints.ai_family = AF_UNSPEC;
  Hints.ai_socktype = SOCK_STREAM;
  Hints.ai_protocol = IPPROTO_TCP;
  addrinfo* Addresses = NULL;
  Result = getaddrinfo(Endpoint.c_str(), PortText.str().c_str(), &Hints, &Addresses);
  if (Result != 0)
  {
    Error = "Host konnte nicht aufgeloest werden: " + Endpoint;
    WSACleanup();
    return false;
  }

  SOCKET ConnectedSocket = INVALID_SOCKET;
  for (addrinfo* Address = Addresses; Address != NULL; Address = Address->ai_next)
  {
    SOCKET Candidate = socket(Address->ai_family, Address->ai_socktype, Address->ai_protocol);
    if (Candidate == INVALID_SOCKET)
      continue;

    if (connect(Candidate, Address->ai_addr, static_cast<int>(Address->ai_addrlen)) == 0)
    {
      ConnectedSocket = Candidate;
      break;
    }
    closesocket(Candidate);
  }
  freeaddrinfo(Addresses);

  if (ConnectedSocket == INVALID_SOCKET)
  {
    Error = "Verbindung zu " + Endpoint + ":" + PortText.str() + " fehlgeschlagen.";
    WSACleanup();
    return false;
  }

  DWORD Timeout = 100;
  setsockopt(ConnectedSocket, SOL_SOCKET, SO_RCVTIMEO,
             reinterpret_cast<const char*>(&Timeout), sizeof(Timeout));
  m_Socket = static_cast<uintptr_t>(ConnectedSocket);
  m_ReceiveBuffer.clear();
  return true;
}

bool CTcpConnection::Write(const std::string& Data, std::string& Error)
{
  SOCKET SocketHandle = static_cast<SOCKET>(m_Socket);
  size_t Written = 0;
  while (Written < Data.size())
  {
    int Result = send(SocketHandle, Data.data() + Written,
                      static_cast<int>(Data.size() - Written), 0);
    if (Result == SOCKET_ERROR || Result == 0)
    {
      Error = "Senden fehlgeschlagen.";
      return false;
    }
    Written += static_cast<size_t>(Result);
  }
  return true;
}

bool CTcpConnection::ReadLine(std::string& Line, int TimeoutMs, std::string& Error)
{
  size_t Newline = m_ReceiveBuffer.find('\n');
  if (Newline != std::string::npos)
  {
    Line = m_ReceiveBuffer.substr(0, Newline);
    m_ReceiveBuffer.erase(0, Newline + 1);
    if (!Line.empty() && Line[Line.size() - 1] == '\r')
      Line.erase(Line.size() - 1);
    return true;
  }

  fd_set ReadSet;
  FD_ZERO(&ReadSet);
  SOCKET SocketHandle = static_cast<SOCKET>(m_Socket);
  FD_SET(SocketHandle, &ReadSet);
  timeval Timeout;
  Timeout.tv_sec = TimeoutMs / 1000;
  Timeout.tv_usec = (TimeoutMs % 1000) * 1000;
  int Ready = select(0, &ReadSet, NULL, NULL, &Timeout);
  if (Ready == 0)
    return false;
  if (Ready == SOCKET_ERROR)
  {
    Error = "Empfang fehlgeschlagen.";
    return false;
  }

  char Buffer[512];
  int Received = recv(SocketHandle, Buffer, sizeof(Buffer), 0);
  if (Received == 0)
  {
    Error = "Gegenstelle hat die Verbindung geschlossen.";
    return false;
  }
  if (Received == SOCKET_ERROR)
  {
    int SocketError = WSAGetLastError();
    if (SocketError == WSAETIMEDOUT || SocketError == WSAEWOULDBLOCK)
      return false;
    Error = "Empfang fehlgeschlagen (" + std::to_string(SocketError) + ").";
    return false;
  }

  m_ReceiveBuffer.append(Buffer, static_cast<size_t>(Received));
  Newline = m_ReceiveBuffer.find('\n');
  if (Newline == std::string::npos)
    return false;

  Line = m_ReceiveBuffer.substr(0, Newline);
  m_ReceiveBuffer.erase(0, Newline + 1);
  if (!Line.empty() && Line[Line.size() - 1] == '\r')
    Line.erase(Line.size() - 1);
  return true;
}

void CTcpConnection::Close()
{
  if (m_Socket != static_cast<uintptr_t>(INVALID_SOCKET))
  {
    closesocket(static_cast<SOCKET>(m_Socket));
    m_Socket = static_cast<uintptr_t>(INVALID_SOCKET);
    WSACleanup();
  }
  m_ReceiveBuffer.clear();
}

CSerialConnection::CSerialConnection()
  : m_Handle(INVALID_HANDLE_VALUE)
{
}

CSerialConnection::~CSerialConnection()
{
  Close();
}

bool CSerialConnection::Open(const std::string& Endpoint, int BaudRate, std::string& Error)
{
  Close();
  std::string DeviceName = Endpoint;
  if (DeviceName.compare(0, 4, "\\\\.\\") != 0)
    DeviceName = "\\\\.\\" + DeviceName;

  HANDLE SerialHandle = CreateFileA(DeviceName.c_str(), GENERIC_READ | GENERIC_WRITE, 0,
                                    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (SerialHandle == INVALID_HANDLE_VALUE)
  {
    Error = "Serieller Port " + Endpoint + " konnte nicht geoeffnet werden.";
    return false;
  }

  DCB Settings = {};
  Settings.DCBlength = sizeof(Settings);
  if (!GetCommState(SerialHandle, &Settings))
  {
    Error = "Serielle Einstellungen konnten nicht gelesen werden.";
    CloseHandle(SerialHandle);
    return false;
  }
  Settings.BaudRate = static_cast<DWORD>(BaudRate);
  Settings.ByteSize = 8;
  Settings.Parity = NOPARITY;
  Settings.StopBits = ONESTOPBIT;
  Settings.fBinary = TRUE;
  if (!SetCommState(SerialHandle, &Settings))
  {
    Error = "Serielle Einstellungen konnten nicht gesetzt werden.";
    CloseHandle(SerialHandle);
    return false;
  }

  COMMTIMEOUTS Timeouts = {};
  Timeouts.ReadIntervalTimeout = 20;
  Timeouts.ReadTotalTimeoutConstant = 20;
  Timeouts.WriteTotalTimeoutConstant = 500;
  if (!SetCommTimeouts(SerialHandle, &Timeouts))
  {
    Error = "Serielle Zeitlimits konnten nicht gesetzt werden.";
    CloseHandle(SerialHandle);
    return false;
  }

  m_Handle = SerialHandle;
  m_ReceiveBuffer.clear();
  PurgeComm(SerialHandle, PURGE_RXCLEAR | PURGE_TXCLEAR);
  return true;
}

bool CSerialConnection::Write(const std::string& Data, std::string& Error)
{
  DWORD Written = 0;
  HANDLE SerialHandle = static_cast<HANDLE>(m_Handle);
  if (!WriteFile(SerialHandle, Data.data(), static_cast<DWORD>(Data.size()), &Written, NULL) ||
      Written != Data.size())
  {
    Error = "Senden ueber den seriellen Port fehlgeschlagen.";
    return false;
  }
  return true;
}

bool CSerialConnection::ReadLine(std::string& Line, int TimeoutMs, std::string& Error)
{
  HANDLE SerialHandle = static_cast<HANDLE>(m_Handle);
  DWORD StartTime = GetTickCount();
  while (GetTickCount() - StartTime < static_cast<DWORD>(TimeoutMs))
  {
    char Character;
    DWORD BytesRead = 0;
    if (!ReadFile(SerialHandle, &Character, 1, &BytesRead, NULL))
    {
      Error = "Empfang ueber den seriellen Port fehlgeschlagen.";
      return false;
    }
    if (BytesRead == 0)
      continue;
    if (Character == '\n')
    {
      if (!m_ReceiveBuffer.empty() && m_ReceiveBuffer[m_ReceiveBuffer.size() - 1] == '\r')
        m_ReceiveBuffer.erase(m_ReceiveBuffer.size() - 1);
      Line.swap(m_ReceiveBuffer);
      return true;
    }
    m_ReceiveBuffer += Character;
  }
  return false;
}

void CSerialConnection::Close()
{
  if (m_Handle != INVALID_HANDLE_VALUE)
  {
    CloseHandle(static_cast<HANDLE>(m_Handle));
    m_Handle = INVALID_HANDLE_VALUE;
  }
  m_ReceiveBuffer.clear();
}
