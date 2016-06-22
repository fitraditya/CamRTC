/*
 * PeerConnectionClient.cpp
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 *
 */

#include <iostream>
#include <utility>

#include "webrtc/modules/video_capture/video_capture_factory.h"
#include "webrtc/media/engine/webrtcvideocapturerfactory.h"
#include "webrtc/media/base/fakevideocapturer.h"

#include "PeerConnectionClient.h"

PeerConnectionClient::PeerConnectionClient(const std::string & iceUrl) : m_peerConnectionFactory(NULL), m_iceUrl(iceUrl) {}

PeerConnectionClient::~PeerConnectionClient() {
  m_peerConnectionFactory = NULL
}

bool PeerConnectionClient::Initialize() {
	m_peerConnectionFactory = webrtc::CreatePeerConnectionFactory();
	return (m_peerConnectionFactory.get() != NULL);
}

const Json::Value PeerConnectionClient::getVideoCapturer() {
  Json::Value root;
	Json::Value capturer;
	std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(webrtc::VideoCaptureFactory::CreateDeviceInfo(0));

  if (info) {
		int num_devices = info->NumberOfDevices();

    for (int i = 0; i < num_devices; ++i) {
			const uint32_t kSize = 256;
			char name[kSize] = {0};
			char id[kSize] = {0};

			if (info->GetDeviceName(i, name, kSize, id, kSize) != -1) {
				capturer.append(name);
			}
		}
	}

  root["data"] = capturer;
	return root;
}
