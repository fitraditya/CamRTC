/*
 * main.cpp
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 *
 */

#include <iostream>

#include "webrtc/base/ssladapter.h"
#include "webrtc/base/thread.h"

#include "HTTPServer.h"

int main(int argc, char* argv[]) {
  const char* address = "0.0.0.0:8000";
  const char* iceUrl = "stun:stun.l.google.com:19302";
  int logLevel = rtc::LERROR;
  PeerConnectionClient rtcClient(iceUrl);

  rtc::LogMessage::LogToDebug((rtc::LoggingSeverity)logLevel);
  rtc::LogMessage::LogTimestamps();
  rtc::LogMessage::LogThreads();

  rtc::Thread* thread = rtc::Thread::Current();
  rtc::InitializeSSL();

  if (rtcClient.Initialize()) {
    rtc::HttpListenServer httpServer;
    rtc::SocketAddress http_addr;
    http_addr.FromString(address);

    if (httpServer.Listen(http_addr) != 0) {
      std::cout << "Cannot initialize HTTP server" << std::endl;
    } else {
      HTTPServer http(&httpServer, &rtcClient);
      std::cout << "HTTP listening on " << http_addr.ToString() << std::endl;

      while(thread->ProcessMessages(10));
    }
  } else {
    std::cout << "Cannot initialize WebRTC client" << std::endl;
  }

  rtc::CleanupSSL();
  return 0;
}
