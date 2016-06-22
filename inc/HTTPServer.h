/*
 * HTTPServer.h
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 *
 */

#include "webrtc/base/httpserver.h"

#include "PeerConnectionClient.h"

class HTTPServer : public sigslot::has_slots<> {
	public:
		HTTPServer(rtc::HttpServer* server, PeerConnectionClient* rtcClient) : m_server(server), m_rtcClient(rtcClient) {
			m_server->SignalHttpRequest.connect(this, &HTTPServer::onRequest);
		}

		void onRequest(rtc::HttpServer*, rtc::HttpServerTransaction* t);

	protected:
		rtc::HttpServer* m_server;
    PeerConnectionClient* m_rtcClient;
};
