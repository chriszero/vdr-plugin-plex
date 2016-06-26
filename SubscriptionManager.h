#ifndef SUBSCRIPTIONMANAGER_H
#define SUBSCRIPTIONMANAGER_H

#include <vdr/tools.h>
#include <vdr/status.h>
#include <string>
#include <iostream>
#include <map>
//#include <mutex>
#include <sstream>
#include "hlsPlayerControl.h"
#include "PVideo.h"

namespace plexclient {

    class cSubscriberStatus : public cStatus {
    protected:
        virtual void Replaying(const cControl *DvbPlayerControl, const char *Name, const char *FileName, bool On);

    public:
        cSubscriberStatus();

        bool PlayerPaused;
        bool PlayerStopped;
        cControl *pControl;
        cVideo *pVideo;
        int Volume;

    };

    class Subscriber {
        friend class SubscriptionManager;

    public:
        std::string m_iCommandId;

        Subscriber() { };

        Subscriber(std::string protocol, std::string host, int port, std::string uuid, std::string commandId);

        std::string GetUuid() {
            return m_sUuid;
        }

        void SendUpdate(std::string msg, bool isNav);

        virtual std::string to_string() {
            return "Subscriber-> Host: " + m_sHost + "; Port: " + std::string(itoa(m_iPort)) + "; Uuid:" + m_sUuid +
                   "; CmdID:" + m_iCommandId;
        }

    private:
        std::string m_sProtocol;
        std::string m_sHost;
        int m_iPort;
        std::string m_sUuid;

        int m_iAge;
    };


    class SubscriptionManager {
    public:
        static SubscriptionManager &GetInstance() {
            static SubscriptionManager instance;
            return instance;
        }

        void AddSubscriber(Subscriber subs);

        void RemoveSubscriber(std::string uuid);

        void UpdateSubscriber(std::string uuid, std::string commandId);

        std::string GetMsg(std::string commandId);

        void Notify();

    private:
        SubscriptionManager();

        cMutexLock m_myLock;
        cMutex m_myMutex;
        std::map<std::string, Subscriber> m_mSubcribers;
        cSubscriberStatus *m_pStatus;
        bool m_bStoppedSent;

        void ReportProgress();

        void NotifyServer();

        void Cleanup();

        std::string GetTimelineXml();
    };

    enum class ActionType { Play, Display };
    struct Action {
        //cVideo video;
        std::shared_ptr<MediaContainer> container;
        ActionType type;
    };

    class ActionManager {
    public:
        static ActionManager &GetInstance() {
            static ActionManager instance;
            return instance;
        }

        void AddAction(Action action);

        Action GetAction();

        bool IsAction();

    private:
        cMutexLock m_myLock;
        cMutex m_myMutex;

        ActionManager();

        Action m_Action;
        bool m_isAction;
    };

}

#endif // SUBSCRIPTIONMANAGER_H
