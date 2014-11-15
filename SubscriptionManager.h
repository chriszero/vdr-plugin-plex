#ifndef SUBSCRIPTIONMANAGER_H
#define SUBSCRIPTIONMANAGER_H

#include <string>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>

#include "player.h"

namespace plexclient
{

class Subscriber
{
	public:
	int m_iCommandId;
	Subscriber() {};
	Subscriber(std::string protocol, std::string host, int port, std::string uuid, int commandId);
	
	std::string GetUuid() {
		return m_sUuid;
	}
	
	void SendUpdate(std::string msg, bool isNav);
	
	virtual std::string to_string() {
		return "Subscriber-> Host: " + m_sHost + "; Port:" + std::to_string(m_iPort) + "; Uuid:" + m_sUuid + "; CmdID:" + std::to_string(m_iCommandId);
	}

private:
	std::string m_sProtocol;
	std::string m_sHost;
	int m_iPort;
	std::string m_sUuid;
	
	int m_iAge;
};
	
	
class SubscriptionManager
{
public:
	static SubscriptionManager& GetInstance() {
		static SubscriptionManager instance;
		return instance;
	}
	void AddSubscriber(Subscriber subs);
	void RemoveSubscriber(std::string uuid);
	std::string GetMsg(std::string commandId);
	void Notify();
	
	private:
	SubscriptionManager();
	std::mutex mutex;
	std::map<std::string, Subscriber> m_mSubcribers;
};

class ActionManager
{
	public:
	static ActionManager& GetInstance() {
		static ActionManager instance;
		return instance;
	}
	void AddAction(std::string file);
	std::string GetAction();
	bool IsAction();
	
	private:
	ActionManager();
	std::string m_sAction = "";
	bool m_isAction = false;
};

}

#endif // SUBSCRIPTIONMANAGER_H
