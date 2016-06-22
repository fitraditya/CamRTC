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
	public:
		PeerConnectionClient(const std::string & iceUrl);
		
    ~PeerConnectionClient();
    
    bool Initialize();

		const Json::Value getVideoCapturer();
  
  protected: 
		rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> m_peerConnectionFactory;
    std::string m_iceUrl;
};
