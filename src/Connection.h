#pragma once

#include <string>
#include <stdint.h>

class CConnection
{
public:
  virtual ~CConnection();
  virtual bool Open(const std::string& Endpoint, int PortOrBaud, std::string& Error) = 0;
  virtual bool Write(const std::string& Data, std::string& Error) = 0;
  virtual bool ReadLine(std::string& Line, int TimeoutMs, std::string& Error) = 0;
  virtual void Close() = 0;
};

class CTcpConnection : public CConnection
{
public:
  CTcpConnection();
  virtual ~CTcpConnection();

  virtual bool Open(const std::string& Endpoint, int Port, std::string& Error);
  virtual bool Write(const std::string& Data, std::string& Error);
  virtual bool ReadLine(std::string& Line, int TimeoutMs, std::string& Error);
  virtual void Close();

private:
  uintptr_t m_Socket;
  std::string m_ReceiveBuffer;
};

class CSerialConnection : public CConnection
{
public:
  CSerialConnection();
  virtual ~CSerialConnection();

  virtual bool Open(const std::string& Endpoint, int BaudRate, std::string& Error);
  virtual bool Write(const std::string& Data, std::string& Error);
  virtual bool ReadLine(std::string& Line, int TimeoutMs, std::string& Error);
  virtual void Close();

private:
  void* m_Handle;
  std::string m_ReceiveBuffer;
};
