/*
 * HTTPServer.cpp
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 *
 */

#include <iostream>

#include "webrtc/base/pathutils.h"

#include "HTTPServer.h"

void HTTPServer::onRequest(rtc::HttpServer*, rtc::HttpServerTransaction* t) {
  std::string host;
  std::string path;
  t->request.getRelativeUri(&host, &path);

  size_t size = 0;
  t->request.document->GetSize(&size);
  t->request.document->Rewind();

  char buffer[size];
  size_t readSize = 0;
  rtc::StreamResult res = t->request.document->ReadAll(&buffer, size, &readSize, NULL);
  std::string body(buffer, readSize);

  if (path == "/device") {
    std::string answer(Json::StyledWriter().write(m_rtcClient->getVideoCapturer()));
    rtc::MemoryStream* mem = new rtc::MemoryStream(answer.c_str(), answer.size());
    t->response.set_success("application/json", mem);
  } else if (path == "/message") {
    Json::Value object;
    Json::Reader reader;
    bool parsingSuccessful = reader.parse(body.c_str(), object);

    if (!parsingSuccessful) {
      std::cout  << "Failed to parsing message: " << reader.getFormattedErrorMessages() << std::endl;
      t->response.set_error(500);
    } else {
      int peerId = 0;
      std::string device;
      std::string type;
      Json::Value data = object["data"];
      rtc::GetIntFromJsonObject(data, "peerId", &peerId);
      rtc::GetStringFromJsonObject(data, "device", &device);
      rtc::GetStringFromJsonObject(data, "type", &type);

      if (!peerId) {
        std::cout  << "Unknown peer id." << std::endl;
        t->response.set_error(500);
      }

      if (type == "offer") {

      } else if (type == "candidate") {

      } else {
        std::cout  << "Unknown message." << std::endl;
        t->response.set_error(500);
      }
    }
  } else {
    rtc::Pathname pathname("content/index.html");
    rtc::FileStream* fs = rtc::Filesystem::OpenFile(pathname, "rb");

    if (fs) {
      t->response.set_success("text/html", fs);
    }
  }

  t->response.setHeader(rtc::HH_CONNECTION, "Close");
  m_server->Respond(t);
}
