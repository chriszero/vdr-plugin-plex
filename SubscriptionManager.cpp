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
#include "PlexHelper.h"
#include "plex.h"

namespace plexclient {

    SubscriptionManager::SubscriptionManager() {
        m_pStatus = new cSubscriberStatus();
        m_bStoppedSent = true;
    }

    void SubscriptionManager::Notify() {
        //dsyslog("[plex]: '%s'", __FUNCTION__);
        Cleanup();
        m_myLock.Lock(&m_myMutex);
        for (std::map<std::string, Subscriber>::iterator it = m_mSubcribers.begin(); it != m_mSubcribers.end(); ++it) {
            Subscriber subs = it->second;
            subs.SendUpdate(GetMsg(subs.m_iCommandId), false);
        }
        NotifyServer();
        ReportProgress();
    }

    void SubscriptionManager::ReportProgress() {
        if (!m_pStatus->pVideo) {
            return;
        }

        try {
            int current, total, speed;
            bool play, forward;
            if (!m_pStatus->PlayerStopped && m_pStatus->pControl) {
                m_pStatus->pControl->GetIndex(current, total);
                current = current / m_pStatus->pControl->FramesPerSecond() * 1000;
                total = total / m_pStatus->pControl->FramesPerSecond() * 1000;
                m_pStatus->pControl->GetReplayMode(play, forward, speed);
            } else {
                return;
            }

            m_pStatus->pControl->GetReplayMode(play, forward, speed);

            std::string state = "playing";
            if (m_pStatus->PlayerStopped) state = "stopped";
            else if (!play) state = "paused";

            std::map<std::string, std::string> queryMap;
            queryMap["key"] = std::to_string(m_pStatus->pVideo->m_iRatingKey);
            queryMap["identifier"] = "com.plexapp.plugins.library";
            queryMap["time"] = std::to_string(current);
            queryMap["state"] = state;

            bool ok;
            auto cSession = m_pStatus->pVideo->m_pServer->MakeRequest(ok, "/:/progress", queryMap);

        } catch (Poco::Exception &) { }
    }

    void SubscriptionManager::NotifyServer() {
        if (m_bStoppedSent && m_pStatus->PlayerStopped) return;
        try {
            int current, total, speed;
            bool play, forward;
            cVideo *pVid = NULL;
            if (!m_pStatus->PlayerStopped && m_pStatus->pControl) {
                m_pStatus->pControl->GetIndex(current, total);
                current = current / m_pStatus->pControl->FramesPerSecond() * 1000;
                total = total / m_pStatus->pControl->FramesPerSecond() * 1000;
                m_pStatus->pControl->GetReplayMode(play, forward, speed);
                pVid = m_pStatus->pVideo;
            } else {
                return;
            }

            PlexServer *pServer;
            if (pVid) {
                pServer = pVid->m_pServer;
            } else if (plexgdm::GetInstance().GetPlexservers().size() > 0) {
                pServer = &plexgdm::GetInstance().GetPlexservers().at(0);
            } else {
                // no plexservers in network
                return;
            }

            std::map<std::string, std::string> queryMap;

            queryMap["containerKey"] = pVid ? pVid->m_sKey : "/library/metadata/900000";
            queryMap["key"] = pVid ? pVid->m_sKey : "/library/metadata/900000";
            queryMap["ratingKey"] = pVid ? std::to_string(pVid->m_iRatingKey) : "900000";

            if (m_pStatus->PlayerStopped) queryMap["state"] = "stopped";
            else if (!play) queryMap["state"] = "paused";
            else queryMap["state"] = "playing";

            queryMap["time"] = std::to_string(current);
            queryMap["duration"] = std::to_string(total);


            Poco::Net::HTTPResponse response;
            bool ok;
            auto cSession = pServer->MakeRequest(ok, "/:/timeline", queryMap);


            m_bStoppedSent = m_pStatus->PlayerStopped ? true : false;
        } catch (Poco::Exception &e) { }

    }

    void SubscriptionManager::AddSubscriber(Subscriber subs) {
        m_myLock.Lock(&m_myMutex);
        m_mSubcribers[subs.GetUuid()] = subs;
    }

    void SubscriptionManager::RemoveSubscriber(std::string uuid) {
        m_myLock.Lock(&m_myMutex);
        if (m_mSubcribers.find(uuid) != m_mSubcribers.end()) {
            m_mSubcribers.erase(uuid);
        }
    }

    void SubscriptionManager::UpdateSubscriber(std::string uuid, std::string commandId) {
        m_myLock.Lock(&m_myMutex);
        if (m_mSubcribers.find(uuid) != m_mSubcribers.end()) {
            m_mSubcribers[uuid].m_iCommandId = commandId;
            //dsyslog("[plex]: '%s'", __FUNCTION__);
        }
    }

    std::string SubscriptionManager::GetMsg(std::string commandId) {
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

    std::string SubscriptionManager::GetTimelineXml() {
        int current = 0, total = 0, speed;
        bool play = false, forward;
        cVideo *pVid = NULL;
        if (!m_pStatus->PlayerStopped && m_pStatus->pControl) {
            m_pStatus->pControl->GetIndex(current, total);
            current = current / m_pStatus->pControl->FramesPerSecond() * 1000;
            total = total / m_pStatus->pControl->FramesPerSecond() * 1000;
            m_pStatus->pControl->GetReplayMode(play, forward, speed);
            pVid = m_pStatus->pVideo;
        }

        std::stringstream msg;

        msg << "<Timeline location=\"navigation\" state=\"";
        if (m_pStatus->PlayerStopped) msg << "stopped";
        else if (!play) msg << "paused";
        else msg << "playing";

        msg << "\" time=\"" << current << "\" type=\"video\"";
        if (!m_pStatus->PlayerStopped) {
            msg << " duration=\"" << total << "\"";
            msg << " seekRange=\"0-" << total << "\"";
            msg << " controllable=\"true\"";
            msg << " machineIdentifier=\"" << (pVid ? pVid->m_pServer->GetUuid() : "") << "\"";
            msg << " protocol=\"http\"";
            msg << " address=\"" << (pVid ? pVid->m_pServer->GetHost() : "") << "\"";
            msg << " port=\"" << (pVid ? pVid->m_pServer->GetPort() : 0) << "\"";
            msg << " guid=\"" << Config::GetInstance().GetUUID() << "\"";
            msg << " containerKey=\"" << (pVid ? pVid->m_sKey : "/library/metadata/900000") << "\"";
            msg << " key=\"" << (pVid ? pVid->m_sKey : "/library/metadata/900000") << "\"";
            msg << " ratingKey=\"" << (pVid ? pVid->m_iRatingKey : 900000) << "\"";
            msg << " volume=\"" << m_pStatus->Volume << "\"";
            msg << " shuffle=\"false\"";
        }
        msg << "/>";
        return msg.str();
    }

    void SubscriptionManager::Cleanup() {
        m_myLock.Lock(&m_myMutex);
        for (std::map<std::string, Subscriber>::iterator it = m_mSubcribers.begin(); it != m_mSubcribers.end(); ++it) {
            Subscriber subs = it->second;
            if (subs.m_iAge > 30) {
                m_mSubcribers.erase(it);
            }
        }
    }

    Subscriber::Subscriber(std::string protocol, std::string host, int port, std::string uuid, std::string commandId) {
        m_sProtocol = protocol;
        int pos = host.find(":");
        m_sHost = host.substr(0, pos - 1);
        m_iPort = port;
        m_sUuid = uuid;
        m_iCommandId = commandId;
        m_iAge = 0;
    }

    void Subscriber::SendUpdate(std::string msg, bool isNav) {
        //dsyslog("[plex]: '%s'", __FUNCTION__);
        ++m_iAge;
        try {
            Poco::Net::HTTPRequest Request(Poco::Net::HTTPRequest::HTTP_POST,
                                           "/:/timeline", Poco::Net::HTTPMessage::HTTP_1_1);

            PlexHelper::AddHttpHeader(Request);

            Poco::Net::HTTPClientSession session(m_sHost, m_iPort);

            Request.setContentLength(msg.length());
            session.sendRequest(Request) << msg;

            Poco::Net::HTTPResponse response;
            session.receiveResponse(response);

        } catch (Poco::Exception &e) { }
    }

    ActionManager::ActionManager() { }

    void ActionManager::AddAction(Action action) {
        m_myLock.Lock(&m_myMutex);
        m_Action = action;
        m_isAction = true;
    }

    Action ActionManager::GetAction() {
        m_myLock.Lock(&m_myMutex);
        m_isAction = false;
        return m_Action;
    }

    bool ActionManager::IsAction() {
        //m_myLock.Lock(&m_myMutex);
        return m_isAction;
    }

    cSubscriberStatus::cSubscriberStatus() {
        PlayerStopped = true;
        pControl = NULL;
    }

    void cSubscriberStatus::Replaying(const cControl *DvbPlayerControl, const char *Name, const char *FileName,
                                      bool On) {
        //dsyslog("[plex]: '%s'", __FUNCTION__);
        PlayerStopped = !On;

        if (PlayerStopped) {
            cMyPlugin::PlayingFile = false;
        }

        pControl = const_cast<cControl *>(DvbPlayerControl);
        cHlsPlayerControl *hlsControl = dynamic_cast<cHlsPlayerControl *>(pControl);
        if (hlsControl) {
            pVideo = &hlsControl->m_Video;
        } else if (cMyPlugin::PlayingFile) {
            pVideo = &cMyPlugin::CurrentVideo;
        } else {
            pVideo = NULL;
            pControl = NULL;
        }

        SubscriptionManager::GetInstance().Notify();
    }

} // Namespace
