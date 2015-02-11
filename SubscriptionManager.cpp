#include "SubscriptionManager.h"
#include "Config.h"

#include <memory>

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/MessageHeader.h>

#include "Plexservice.h"
#include "plexgdm.h"

namespace plexclient
{

SubscriptionManager::SubscriptionManager()
{
	m_pStatus = new cSubscriberStatus();
	m_bStoppedSent = true;
}

void SubscriptionManager::Notify()
{
	//dsyslog("[plex]: '%s'", __FUNCTION__);
	Cleanup();
	m_myLock.Lock(&m_myMutex);
	for(std::map<std::string, Subscriber>::iterator it = m_mSubcribers.begin() ; it != m_mSubcribers.end(); ++it) {
		Subscriber subs = it->second;
		subs.SendUpdate(GetMsg(subs.m_iCommandId), false);
	}
	NotifyServer();
}

void SubscriptionManager::NotifyServer()
{
	if(m_bStoppedSent && m_pStatus->PlayerStopped) return;
	try {
		int current, total, speed;
		bool play, forward;
		Video *pVid = NULL;
		if(!m_pStatus->PlayerStopped && m_pStatus->pControl) {
			m_pStatus->pControl->GetIndex(current, total);
			current = current / m_pStatus->pControl->FramesPerSecond() * 1000;
			total = total / m_pStatus->pControl->FramesPerSecond() * 1000;
			m_pStatus->pControl->GetReplayMode(play, forward, speed);
			pVid = m_pStatus->pVideo;
		} else {
			return;
		}
		
		std::string server;
		int port;
		if(pVid) {
			server = pVid->m_pServer->GetIpAdress();
			port = pVid->m_pServer->GetPort();
		} else if ( plexgdm::GetInstance().GetPlexservers().size() > 0) {
			server = plexgdm::GetInstance().GetPlexservers().at(0).GetIpAdress();
			port = plexgdm::GetInstance().GetPlexservers().at(0).GetPort();
		} else {
			// no plexservers in network
			return;
		}

		std::stringstream uri;
		uri << "/:/timeline";
		uri << "?containerKey=" << Plexservice::encode(pVid ? pVid->m_sKey : "/library/metadata/900000");
		uri << "&key=" << Plexservice::encode(pVid ? pVid->m_sKey : "/library/metadata/900000");
		uri << "&ratingKey=" << (pVid ? pVid->m_iRatingKey : 900000);
		uri << "&state=";
		if (m_pStatus->PlayerStopped)	uri << "stopped";
		else if(!play) 					uri << "paused";
		else 							uri << "playing";
		uri << "&time=" << current;
		uri << "&duration=" << total;

		Poco::Net::HTTPRequest Request(Poco::Net::HTTPRequest::HTTP_GET, uri.str(), Poco::Net::HTTPMessage::HTTP_1_1);

		Request.add("User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_8_2) AppleWebKit/537.17 (KHTML, like Gecko) Chrome/24.0.1312.52 Safari/537.17");
		Request.add("X-Plex-Client-Identifier", Config::GetInstance().GetUUID());
		Request.add("X-Plex-Device", "PC");
		Request.add("X-Plex-Device-Name", Config::GetInstance().GetHostname());
		Request.add("X-Plex-Language", Config::GetInstance().GetLanguage());
		Request.add("X-Plex-Model", "Linux");
		Request.add("X-Plex-Platform", "VDR");
		Request.add("X-Plex-Product", "plex-vdr");
		Request.add("X-Plex-Provides", "player");
		Request.add("X-Plex-Version", "0.0.1a");
		
		Poco::Net::HTTPClientSession session(server, port);

		session.sendRequest(Request);
		Poco::Net::HTTPResponse response;
		session.receiveResponse(response);

		if(m_pStatus->PlayerStopped) {
			m_bStoppedSent = true;
		} else {
			m_bStoppedSent = false;
		}
	} catch (Poco::Exception& e) {
		std::cout << e.displayText() << std::endl;
	}

}

void SubscriptionManager::AddSubscriber(Subscriber subs)
{
	m_myLock.Lock(&m_myMutex);
	m_mSubcribers[subs.GetUuid()] = subs;
}

void SubscriptionManager::RemoveSubscriber(std::string uuid)
{
	m_myLock.Lock(&m_myMutex);
	if(m_mSubcribers.find(uuid) != m_mSubcribers.end()) {
		m_mSubcribers.erase(uuid);
	}
}

void SubscriptionManager::UpdateSubscriber(std::string uuid, std::string commandId)
{
	m_myLock.Lock(&m_myMutex);
	if(m_mSubcribers.find(uuid) != m_mSubcribers.end()) {
		m_mSubcribers[uuid].m_iCommandId = commandId;
		//dsyslog("[plex]: '%s'", __FUNCTION__);
	}
}

std::string SubscriptionManager::GetMsg(std::string commandId)
{
	//dsyslog("[plex]: '%s'", __FUNCTION__);
	std::stringstream msg;
	msg << "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
	    "<MediaContainer commandID=\"" << commandId << "\" location=\"navigation\">";

	msg << "<Timeline location=\"navigation\" state=\"stopped\" time=\"0\" type=\"photo\" />";
	msg << "<Timeline location=\"navigation\" state=\"stopped\" time=\"0\" type=\"music\" />";

	msg << GetTimelineXml();

	msg << "</MediaContainer>";
	return msg.str();
}

std::string SubscriptionManager::GetTimelineXml()
{
	int current, total, speed;
	bool play, forward;
	Video *pVid = NULL;
	if(!m_pStatus->PlayerStopped && m_pStatus->pControl) {
		m_pStatus->pControl->GetIndex(current, total);
		current = current / m_pStatus->pControl->FramesPerSecond() * 1000;
		total = total / m_pStatus->pControl->FramesPerSecond() * 1000;
		m_pStatus->pControl->GetReplayMode(play, forward, speed);
		pVid = m_pStatus->pVideo;
	}

	std::stringstream msg;

	msg << "<Timeline location=\"navigation\" state=\"";
	if (m_pStatus->PlayerStopped)	msg << "stopped";
	else if(!play) 				msg << "paused";
	else 						msg << "playing";

	msg << "\" time=\"" << current << "\" type=\"video\"";

	msg << " duration=\"" << total << "\"";
	msg << " seekRange=\"0-" << total << "\"";
	msg << " controllable=\"true\"";
	msg << " machineIdentifier=\"" << (pVid ? pVid->m_pServer->GetUuid()	 : "") << "\"";
	msg << " protocol=\"http\"";
	msg << " address=\"" << (pVid ? pVid->m_pServer->GetIpAdress() : "") << "\"";
	msg << " port=\"" << (pVid ? pVid->m_pServer->GetPort() : 0) << "\"";
	msg << " guid=\"" << Config::GetInstance().GetUUID() << "\"";
	msg << " containerKey=\"" << (pVid ? pVid->m_sKey : "/library/metadata/900000") << "\"";
	msg << " key=\"" << (pVid ? pVid->m_sKey : "/library/metadata/900000") << "\"";
	msg << " ratingKey=\"" << (pVid ? pVid->m_iRatingKey : 900000) << "\"";
	msg << " volume=\"" << m_pStatus->Volume << "\"";
	msg << " shuffle=\"false\"";

	msg <<  "/>";
	return msg.str();
}

void SubscriptionManager::Cleanup()
{
	m_myLock.Lock(&m_myMutex);
	for(std::map<std::string, Subscriber>::iterator it = m_mSubcribers.begin() ; it != m_mSubcribers.end(); ++it) {
		Subscriber subs = it->second;
		if(subs.m_iAge > 30) {
			m_mSubcribers.erase(it);
		}
	}
}

Subscriber::Subscriber(std::string protocol, std::string host, int port, std::string uuid, std::string commandId)
{
	m_sProtocol = protocol;
	int pos = host.find(":");
	m_sHost = host.substr(0,pos-1);
	m_iPort = port;
	m_sUuid = uuid;
	m_iCommandId = commandId;
	m_iAge = 0;
}

void Subscriber::SendUpdate(std::string msg, bool isNav)
{
	//dsyslog("[plex]: '%s'", __FUNCTION__);
	++m_iAge;
	try {
		Poco::Net::HTTPRequest Request(Poco::Net::HTTPRequest::HTTP_POST,
		                               "/:/timeline", Poco::Net::HTTPMessage::HTTP_1_1);

		Request.add("User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_8_2) AppleWebKit/537.17 (KHTML, like Gecko) Chrome/24.0.1312.52 Safari/537.17");

		Request.add("X-Plex-Client-Identifier", Config::GetInstance().GetUUID());
		Request.add("X-Plex-Device", "PC");
		Request.add("X-Plex-Device-Name", Config::GetInstance().GetHostname());
		Request.add("X-Plex-Language", Config::GetInstance().GetLanguage());
		Request.add("X-Plex-Model", "Linux");
		Request.add("X-Plex-Platform", "VDR");
		Request.add("X-Plex-Product", "plex-vdr");
		Request.add("X-Plex-Provides", "player");
		Request.add("X-Plex-Version", "0.0.1a");

		Poco::Net::HTTPClientSession session(m_sHost, m_iPort);

		Request.setContentLength(msg.length());
		session.sendRequest(Request) << msg;

		Poco::Net::HTTPResponse response;
		session.receiveResponse(response);

	} catch (Poco::Exception& e) {}
}

ActionManager::ActionManager() {}

void ActionManager::AddAction(Video* video)
{
	m_myLock.Lock(&m_myMutex);
	m_Action = video;
	m_isAction = true;
}

Video* ActionManager::GetAction()
{
	m_myLock.Lock(&m_myMutex);
	m_isAction = false;
	return m_Action;
}

bool ActionManager::IsAction()
{
	//m_myLock.Lock(&m_myMutex);
	return m_isAction;
}

cSubscriberStatus::cSubscriberStatus()
{
	PlayerStopped = true;
	pControl = NULL;
}

void cSubscriberStatus::Replaying(const cControl* DvbPlayerControl, const char* Name, const char* FileName, bool On)
{
	//dsyslog("[plex]: '%s'", __FUNCTION__);
	PlayerStopped = !On;
	pControl = const_cast<cControl*>(DvbPlayerControl);
	cHlsPlayerControl* hlsControl = dynamic_cast<cHlsPlayerControl*>(pControl);
	if(hlsControl) {
		pVideo = hlsControl->m_pVideo;
	} else {
		pVideo = NULL;
	}

	SubscriptionManager::GetInstance().Notify();
}

} // Namespace
