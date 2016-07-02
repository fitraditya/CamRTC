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

PeerConnectionClient::PeerConnectionClient(const std::string& iceUrl) : m_peerConnectionFactory(NULL), m_iceUrl(iceUrl) {}

PeerConnectionClient::~PeerConnectionClient() {
  m_peerConnectionFactory = NULL;
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

cricket::VideoCapturer* PeerConnectionClient::OpenVideoCaptureDevice(const std::string& device) {
  cricket::VideoCapturer* capturer = NULL;

  std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(webrtc::VideoCaptureFactory::CreateDeviceInfo(0));

  if (info) {
    int numDevices = info->NumberOfDevices();

    for (int i = 0; i < numDevices; ++i) {
      const uint32_t size = 256;
      char name[size] = {0};
      char id[size] = {0};

      if (info->GetDeviceName(i, name, size, id, size) != -1) {
        if (device == name) {
          cricket::WebRtcVideoDeviceCapturerFactory factory;
          capturer = factory.Create(cricket::Device(name, 0));
        }
      }
    }
  }

  return capturer;
}


bool PeerConnectionClient::AddStreams(webrtc::PeerConnectionInterface* peerConnection, const std::string& device) {
  bool ret = false;

  cricket::VideoCapturer* capturer = OpenVideoCaptureDevice(device);

  if (!capturer) {
    std::cout << "Can not open capturer: " << device << std::endl;
  } else {
    rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> source = m_peerConnectionFactory->CreateVideoSource(capturer, NULL);
    rtc::scoped_refptr<webrtc::VideoTrackInterface> videoTrack(m_peerConnectionFactory->CreateVideoTrack("video_label", source));
    rtc::scoped_refptr<webrtc::MediaStreamInterface> stream = m_peerConnectionFactory->CreateLocalMediaStream("stream_label");

    if (!stream.get()) {
      std::cout << "Can not create stream." << std::endl;
    } else {
      if (!stream->AddTrack(videoTrack)) {
        std::cout << "Adding track to peer connection failed." << std::endl;
      } else if (!peerConnection->AddStream(stream)) {
        std::cout << "Adding stream to peer connection failed." << std::endl;
      } else {
        ret = true;
      }
    }
  }

  return ret;
}

std::pair<rtc::scoped_refptr<webrtc::PeerConnectionInterface>, PeerConnectionClient::PeerConnectionObserver* > PeerConnectionClient::CreatePeerConnection(const std::string& device) {
  webrtc::PeerConnectionInterface::RTCConfiguration config;
  webrtc::PeerConnectionInterface::IceServer server;

  server.uri = "stun:" + m_iceUrl;
  server.username = "";
  server.password = "";
  config.servers.push_back(server);

  PeerConnectionObserver* observer = PeerConnectionObserver::Create();
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> peerConnection = m_peerConnectionFactory->CreatePeerConnection(config, NULL, NULL, NULL, observer);

  if (!peerConnection) {
    std::cout << "Create peer connection failed." << std::endl;
  } else {
    observer->setPeerConnection(peerConnection);

    if (!this->AddStreams(peerConnection, device)) {
      peerConnection.release();
    }
  }

  return std::pair<rtc::scoped_refptr<webrtc::PeerConnectionInterface>, PeerConnectionObserver* >(peerConnection, observer);
}

const std::string PeerConnectionClient::setOffer(const int& peerId, const std::string& device, const std::string& message) {
  Json::Reader reader;
  Json::Value object;
  std::string answer;

  std::pair<rtc::scoped_refptr<webrtc::PeerConnectionInterface>, PeerConnectionObserver* > peerConnection = this->CreatePeerConnection(device);

  if (!peerConnection.first) {
    std::cout << "Failed to initialize peer connection." << std::endl;
  } else {
    m_peerConnectionMap.insert(std::pair<int, rtc::scoped_refptr<webrtc::PeerConnectionInterface> >(peerId, peerConnection.first));
    m_peerConnectionObserverMap.insert(std::pair<int, PeerConnectionObserver* >(peerId, peerConnection.second));

    if (!reader.parse(message, object)) {
      return "[ERROR] Unknown message.";
    }

    std::string type;
    std::string sdp;

    if (!rtc::GetStringFromJsonObject(object, "type", &type) || !rtc::GetStringFromJsonObject(object, "sdp", &sdp)) {
      return "[ERROR] Can not parse message.";
    }

    webrtc::SessionDescriptionInterface* sessionDescription(webrtc::CreateSessionDescription(type, sdp, NULL));

    if (!sessionDescription) {
      return "[ERROR] Can not parse message.";
    }

    std::map<int, rtc::scoped_refptr<webrtc::PeerConnectionInterface> >::iterator it = m_peerConnectionMap.find(peerId);

    if (it != m_peerConnectionMap.end()) {
      rtc::scoped_refptr<webrtc::PeerConnectionInterface> peerConnection = it->second;
      peerConnection->SetRemoteDescription(SetSessionDescriptionObserver::Create(peerConnection, sessionDescription->type()), sessionDescription);
    }

    peerConnection.first->CreateAnswer(CreateSessionDescriptionObserver::Create(peerConnection.first), NULL);

    int count = 10;

    while ((peerConnection.first->local_description() == NULL) && (--count > 0)) {
      rtc::Thread::Current()->ProcessMessages(10);
    }

    const webrtc::SessionDescriptionInterface* description = peerConnection.first->local_description();

    if (description) {
      std::string sdpAnswer;
      Json::Value message;

      description->ToString(&sdpAnswer);

      message["type"] = description->type();
      message["sdp"] = sdpAnswer;

      Json::StyledWriter writer;
      answer = writer.write(message);
    }
  }

  return answer;
}

void PeerConnectionClient::PeerConnectionObserver::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
  std::string sdp;

  if (!candidate->ToString(&sdp)) {
    std::cout << "Failed to serialize candidate." << std::endl;
  } else {
    Json::Value object;
    object["sdpMid"] = candidate->sdp_mid();
    object["sdpMLineIndex"] = candidate->sdp_mline_index();
    object["candidate"] = sdp;
    m_iceCandidateList.append(object);
  }
}
