#include "SubscriptionManager.h"
#include "Config.h"

#include <memory>

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/MessageHeader.h>

namespace plexclient
{

SubscriptionManager::SubscriptionManager()
{
	
}

void SubscriptionManager::Notify() {
	mutex.lock();
	
	//TODO: Implement
	for(auto subs : m_mSubcribers) {
		subs.second.SendUpdate(GetMsg(std::to_string(subs.second.m_iCommandId)), false);
	}
	
	mutex.unlock();
}

void SubscriptionManager::AddSubscriber(Subscriber subs) {
	mutex.lock();
	m_mSubcribers[subs.GetUuid()] = subs;
	std::cout << "AddSubscriber: " << subs.to_string() << std::endl;
	mutex.unlock();
}

void SubscriptionManager::RemoveSubscriber(std::string uuid) {
	mutex.lock();
	if(m_mSubcribers.find(uuid) != m_mSubcribers.end()) {
		m_mSubcribers.erase(uuid);
	}
	mutex.unlock();
}

std::string SubscriptionManager::GetMsg(std::string commandId) {
	PlayerGetCurrentPosition();
	int time = PlayerCurrent;
	
	bool paused = PlayerPaused;
	
	std::stringstream msg;
	msg << "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		   "<MediaContainer commandID=\"" << commandId << "\""
		   " location=\"navigation\">";
	
	msg << "<Timeline location=\"navigation\" state=\"";
	//if(paused) 		msg << "paused";
	//else if (paused) 	msg	<< "paused";
	 				msg << "stopped";
	msg << "\" time=\"" << time << "\" type=\"video\" />";
	msg << "<Timeline location=\"navigation\" state=\"stopped\" time=\"0\" type=\"photo\" />";
	msg << "<Timeline location=\"navigation\" state=\"stopped\" time=\"0\" type=\"music\" />";
	
	msg << "</MediaContainer>";
	return msg.str();
}

Subscriber::Subscriber(std::string protocol, std::string host, int port, std::string uuid, int commandId) {
	m_sProtocol = protocol;
	m_sHost = host;
	m_iPort = port;
	m_sUuid = uuid;
	m_iCommandId = commandId;
}

void Subscriber::SendUpdate(std::string msg, bool isNav) {
	++m_iAge;
	
	Poco::Net::HTTPRequest *pRequest = new Poco::Net::HTTPRequest(Poco::Net::HTTPRequest::HTTP_POST,
	        "/:/timeline", Poco::Net::HTTPMessage::HTTP_1_1);

	pRequest->add("User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_8_2) AppleWebKit/537.17 (KHTML, like Gecko) Chrome/24.0.1312.52 Safari/537.17");

	pRequest->add("X-Plex-Client-Capabilities", "protocols=shoutcast,http-video;videoDecoders=h264{profile:high&resolution:1080&level:51};audioDecoders=mp3,aac");
	pRequest->add("X-Plex-Client-Identifier", Config::GetInstance().GetUUID());
	pRequest->add("X-Plex-Device", "PC");
	pRequest->add("X-Plex-Device-Name", Config::GetInstance().GetHostname());
	pRequest->add("X-Plex-Language", Config::GetInstance().GetLanguage());
	pRequest->add("X-Plex-Model", "Linux");
	pRequest->add("X-Plex-Platform", "VDR");
	pRequest->add("X-Plex-Product", "plex-vdr");
	pRequest->add("X-Plex-Provides", "player");
	pRequest->add("X-Plex-Version", "0.0.1a");
	
	auto session = new Poco::Net::HTTPClientSession(m_sHost, m_iPort);
	std::ostream& oustr = session->sendRequest(*pRequest);
	oustr << msg;
}

ActionManager::ActionManager() {}

void ActionManager::AddAction(std::string file) {
	m_sAction = file;
	m_isAction = true;
}

std::string ActionManager::GetAction() {
	m_isAction = false;
	return m_sAction;
}

bool ActionManager::IsAction() {
	return m_isAction;
}

}

