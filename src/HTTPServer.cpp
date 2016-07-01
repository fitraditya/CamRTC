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
    std::cout << "body: " << body << std::endl;
    Json::Value object;
    Json::Reader reader;
    bool parsingSuccessful = reader.parse(body.c_str(), object);

    if (!parsingSuccessful) {
      std::cout  << "Failed to parsing message: " << reader.getFormattedErrorMessages() << std::endl;
      t->response.set_error(500);
    } else {
      std::string type;
      std::string message;
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
