/*
 * PeerConnectionClient.h
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 *
 */

#include <string>

#include "webrtc/api/peerconnection.h"
#include "webrtc/base/logging.h"
#include "webrtc/base/json.h"

class PeerConnectionClient {
  class CreateSessionDescriptionObserver : public webrtc::CreateSessionDescriptionObserver {
    public:
      static CreateSessionDescriptionObserver* Create(webrtc::PeerConnectionInterface* peerConnection) {
        return new rtc::RefCountedObject<CreateSessionDescriptionObserver>(peerConnection);
      }

      virtual void OnSuccess(webrtc::SessionDescriptionInterface* description) {
        m_peerConnection->SetLocalDescription(SetSessionDescriptionObserver::Create(m_peerConnection, description->type()), description);
      }

      virtual void OnFailure(const std::string& error) {
        std::cout << "Error creating session description: " << error << std::endl;
      }

    protected:
      CreateSessionDescriptionObserver(webrtc::PeerConnectionInterface* peerConnection) : m_peerConnection(peerConnection) {};

    private:
      webrtc::PeerConnectionInterface* m_peerConnection;
  };

  class SetSessionDescriptionObserver : public webrtc::SetSessionDescriptionObserver {
    public:
      static SetSessionDescriptionObserver* Create(webrtc::PeerConnectionInterface* peerConnection, const std::string& type) {
        return new rtc::RefCountedObject<SetSessionDescriptionObserver>(peerConnection, type);
      }

      virtual void OnSuccess() {
        std::string sdp;
        m_peerConnection->local_description()->ToString(&sdp);
      }

      virtual void OnFailure(const std::string& error) {
        std::cout << "Error setting session description: " << error << std::endl;
      }

    protected:
      SetSessionDescriptionObserver(webrtc::PeerConnectionInterface* peerConnection, const std::string& type) : m_peerConnection(peerConnection), m_type(type) {};

    private:
      webrtc::PeerConnectionInterface* m_peerConnection;
      std::string m_type;
  };

  class PeerConnectionObserver : public webrtc::PeerConnectionObserver {
    public:
      static PeerConnectionObserver* Create() {
        return new PeerConnectionObserver();
      }

      void setPeerConnection(webrtc::PeerConnectionInterface* peerConnection) {
        m_peerConnection = peerConnection;
      };

      Json::Value getIceCandidateList() {
        return m_iceCandidateList;
      };

      virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate);
      virtual void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState) {}
      virtual void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState) {}
      virtual void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState) {}
      virtual void OnAddStream(webrtc::MediaStreamInterface* stream) {}
      virtual void OnRemoveStream(webrtc::MediaStreamInterface* stream) {}
      virtual void OnDataChannel(webrtc::DataChannelInterface* channel) {}
      virtual void OnRenegotiationNeeded() {
        LOG(LERROR) << __PRETTY_FUNCTION__;
      }

    protected:
      PeerConnectionObserver() : m_peerConnection(NULL) {};

    private:
      webrtc::PeerConnectionInterface* m_peerConnection;
      Json::Value m_iceCandidateList;
  };

  public:
    PeerConnectionClient(const std::string& iceUrl);

    ~PeerConnectionClient();

    bool Initialize();

    const Json::Value getVideoCapturer();
    const std::string setOffer(const int& peerId, const std::string& device, const std::string& sdp);

  protected:
    cricket::VideoCapturer* OpenVideoCaptureDevice(const std::string& device);
    bool AddStreams(webrtc::PeerConnectionInterface* peerConnection, const std::string& device);
    std::pair<rtc::scoped_refptr<webrtc::PeerConnectionInterface>, PeerConnectionClient::PeerConnectionObserver* > CreatePeerConnection(const std::string& device);

  protected:
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> m_peerConnectionFactory;
    std::map<int, rtc::scoped_refptr<webrtc::PeerConnectionInterface> > m_peerConnectionMap;
    std::map<int, PeerConnectionObserver* >  m_peerConnectionObserverMap;
    std::string m_iceUrl;
    Json::Value m_iceCandidateList;
};
