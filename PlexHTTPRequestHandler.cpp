#include "PlexHTTPRequestHandler.h"
#include <vdr/remote.h>
#include <vdr/keys.h>
#include <unistd.h>

#include <Poco/SharedPtr.h>

#include "hlsPlayerControl.h"
#include "services.h"

namespace plexclient {

    void PlexHTTPRequestHandler::AddHeaders(Poco::Net::HTTPServerResponse &response,
                                            Poco::Net::HTTPServerRequest &request) {
        if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_OPTIONS) {
            response.setContentType("text/plain");

            response.add("X-Plex-Client-Identifier", Config::GetInstance().GetUUID());
            response.add("Connection", "Close");
            response.add("Access-Control-Max-Age", "1209600");
            response.add("Access-Control-Allow-Origin", "*");
            response.add("Access-Control-Allow-Methods", "POST, GET, OPTIONS, DELETE, PUT, HEAD");
            response.add("Access-Control-Allow-Headers",
                         "x-plex-version, x-plex-platform-version, "
                                 "x-plex-username, x-plex-client-identifier, "
                                 "x-plex-target-client-identifier, x-plex-device-name, "
                                 "x-plex-platform, x-plex-product, accept, x-plex-device");
        } else {
            response.setContentType("application/x-www-form-urlencoded");

            response.add("Access-Control-Allow-Origin", "*");
            response.add("X-Plex-Version", VERSION);
            response.add("X-Plex-Client-Identifier", Config::GetInstance().GetUUID());
            response.add("X-Plex-Product", DESCRIPTION);
            response.add("X-Plex-Device-Name", Config::GetInstance().GetHostname());
            response.add("X-Plex-Platform", "VDR");
            response.add("X-Plex-Model", "Linux");
            response.add("X-Plex-Device", "PC");
            response.add("X-Plex-Username", Config::GetInstance().GetUsername());
        }
        // header.MessageHeader(header);
    }

    std::map<std::string, std::string> PlexHTTPRequestHandler::ParseQuery(std::string query) {
        std::map<std::string, std::string> querymap;
        Poco::StringTokenizer queryTokens(query, "&");
        for (Poco::StringTokenizer::Iterator token = queryTokens.begin(); token != queryTokens.end(); token++) {
            int pos = token->find("=");
            querymap[token->substr(0, pos)] = token->substr(pos + 1, std::string::npos);
        }
        return querymap;
    }

    std::string PlexHTTPRequestHandler::GetOKMsg() {
        return "<?xml version=\"1.0\" encoding=\"utf-8\"?> <Response code=\"200\" status=\"OK\" />";
    }

    void PlexHTTPRequestHandler::handleRequest(Poco::Net::HTTPServerRequest &request,
                                               Poco::Net::HTTPServerResponse &response) {
        response.send() << " "; // Stream must not be empty!
        response.setStatus(Poco::Net::HTTPResponse::HTTP_REASON_NOT_FOUND);
    }

    void PlexHTTPRequestHandler::UpdateCommandId(Poco::Net::HTTPServerRequest &request) {
        Poco::URI uri(request.getURI());
        std::map<std::string, std::string> query = ParseQuery(uri.getQuery()); // port=32400&commandID=0&protocol=http

        std::string uuid = request.get("X-Plex-Client-Identifier", "");
        if (uuid.length() != 0) {
            std::string command = query["commandID"];
            SubscriptionManager::GetInstance().UpdateSubscriber(uuid, command);
        }
    }

    void SubscribeRequestHandler::handleRequest(Poco::Net::HTTPServerRequest &request,
                                                Poco::Net::HTTPServerResponse &response) {
        UpdateCommandId(request);
        Poco::URI uri(request.getURI());
        std::map<std::string, std::string> query = ParseQuery(uri.getQuery());

        if (query.find("wait") != query.end() && atoi(query["wait"].c_str()) == 1) {
            usleep(900 * 1000);
        }

        if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_OPTIONS) {
            AddHeaders(response, request);
            response.send() << " "; // Stream must not be empty!
            response.setStatus(Poco::Net::HTTPResponse::HTTP_REASON_OK);
            return;
        }

        // parse query
        if (request.getURI().find("/subscribe") != std::string::npos) {
            Subscribe(request);
            AddHeaders(response, request);
            response.send() << GetOKMsg();
            response.setStatus(Poco::Net::HTTPResponse::HTTP_REASON_OK);
        } else if (request.getURI().find("/unsubscribe") != std::string::npos) {
            Unsubscribe(request);
            AddHeaders(response, request);
            response.send() << GetOKMsg();
            response.setStatus(Poco::Net::HTTPResponse::HTTP_REASON_OK);
        } else if (request.getURI().find("/poll") != std::string::npos) {
            response.add("X-Plex-Client-Identifier", Config::GetInstance().GetUUID());
            response.add("Access-Control-Expose-Headers", "X-Plex-Client-Identifier");
            response.add("Access-Control-Allow-Origin", "*");
            response.setContentType("text/xml");
            response.send() << SubscriptionManager::GetInstance().GetMsg(query["commandID"]);
            response.setStatus(Poco::Net::HTTPResponse::HTTP_REASON_OK);
        }
    }

    void SubscribeRequestHandler::Subscribe(Poco::Net::HTTPServerRequest &request) {
        Poco::URI uri(request.getURI());
        std::map<std::string, std::string> query = ParseQuery(uri.getQuery()); // port=32400&commandID=0&protocol=http

        std::string uuid = request.get("X-Plex-Client-Identifier", "");
        if (uuid.length() != 0) {
            int port = atoi(query["port"].c_str());
            std::string command = query["commandID"];

            SubscriptionManager::GetInstance().AddSubscriber(
                    Subscriber(query["protocol"], request.clientAddress().host().toString(), port, uuid, command));
        }
    }

    void SubscribeRequestHandler::Unsubscribe(Poco::Net::HTTPServerRequest &request) {
        std::string uuid = request.get("X-Plex-Client-Identifier", "");
        if (uuid.length() != 0) {
            SubscriptionManager::GetInstance().RemoveSubscriber(uuid);
        }
    }

    void ResourceRequestHandler::handleRequest(Poco::Net::HTTPServerRequest &request,
                                               Poco::Net::HTTPServerResponse &response) {
        UpdateCommandId(request);
        AddHeaders(response, request);

        std::ostream &ostr = response.send();
        ostr << "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                "<MediaContainer>"
                "<Player title=\"" << Config::GetInstance().GetHostname() << "\""
                " protocol=\"plex\""
                " protocolVersion=\"1\""
                " protocolCapabilities=\"navigation,playback,timeline,mirror\""
                " machineIdentifier=\"" << Config::GetInstance().GetUUID() << "\""
                " product=\"" << DESCRIPTION << "\""
                " platform=\"Linux\""
                " platformVersion=\"" << VERSION << "\""
                " deviceClass=\"HTPC\""
                "/> </MediaContainer>";

        response.setStatus(Poco::Net::HTTPResponse::HTTP_REASON_OK);

        // dsyslog("[plex]Resources Response sent...");
    }

    void PlayerRequestHandler::handleRequest(Poco::Net::HTTPServerRequest &request,
                                             Poco::Net::HTTPServerResponse &response) {
        try {
            UpdateCommandId(request);

            Poco::URI uri(request.getURI());
            std::map<std::string, std::string> query = ParseQuery(uri.getQuery());

            if (query.find("wait") != query.end() && atoi(query["wait"].c_str()) == 1) {
                usleep(900 * 1000);
            }

            if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_OPTIONS) {
                AddHeaders(response, request);
                response.send() << " "; // Stream must not be empty!
                return;
            }

            if (request.getURI().find("/poll") != std::string::npos) {
                response.add("X-Plex-Client-Identifier", Config::GetInstance().GetUUID());
                response.add("Access-Control-Expose-Headers", "X-Plex-Client-Identifier");
                response.add("Access-Control-Allow-Origin", "*");
                response.setContentType("text/xml");
                std::ostream &ostr = response.send();
                ostr << SubscriptionManager::GetInstance().GetMsg(query["commandID"]);

            } else if (request.getURI().find("/navigation") != std::string::npos) {
                if (request.getURI().find("/moveUp") != std::string::npos) {
                    cRemote::Put(kUp);
                } else if (request.getURI().find("/moveDown") != std::string::npos) {
                    cRemote::Put(kDown);
                } else if (request.getURI().find("/moveLeft") != std::string::npos) {
                    cRemote::Put(kLeft);
                } else if (request.getURI().find("/moveRight") != std::string::npos) {
                    cRemote::Put(kRight);
                } else if (request.getURI().find("/select") != std::string::npos) {
                    cRemote::Put(kOk);
                } else if (request.getURI().find("/home") != std::string::npos) {
                    cRemote::Put(kMenu);
                } else if (request.getURI().find("/back") != std::string::npos) {
                    cRemote::Put(kBack);
                }
                response.send() << GetOKMsg();

            } else if (request.getURI().find("/playback") != std::string::npos) {
                if (request.getURI().find("/playback/seekTo") != std::string::npos) {
                    cHlsPlayerControl *control = dynamic_cast<cHlsPlayerControl *>(cControl::Control(true));
                    if (query.find("offset") != query.end()) {
                        int offset = atoi(query["offset"].c_str()) / 1000;
                        if (control) {
                            isyslog("[plex] Seeking to %d", offset);
                            control->SeekTo(offset);
                        } else if (cMyPlugin::PlayingFile) {
                            cPlugin *mpvPlugin = cPluginManager::GetPlugin("mpv");
                            if (mpvPlugin) {
                                Mpv_Seek seekData;
                                seekData.SeekAbsolute = offset;
                                seekData.SeekRelative = 0;
                                mpvPlugin->Service(MPV_SEEK, &seekData);
                            }
                        }
                    }
                } else if (request.getURI().find("/playback/playMedia") != std::string::npos) {
                    AddHeaders(response, request);
                    std::string protocol = query["protocol"];
                    std::string address = query["address"];
                    std::string port = query["port"];
                    std::string key = query["key"];

                    std::string fullUrl = protocol + "://" + address + ":" + port + key; // Metainfo
                    auto Cont = Plexservice::GetMediaContainer(fullUrl);

                    // Check for video
                    if (Cont && Cont->m_vVideos.size() > 0) {
                        // MUSS im Maintread des Plugins/VDR gestartet werden
                        if (query.find("offset") != query.end()) {
                            Cont->m_vVideos[0].m_iMyPlayOffset = atoi(query["offset"].c_str()) / 1000;
                            if (Cont->m_vVideos[0].m_iMyPlayOffset == 0) {
                                Cont->m_vVideos[0].m_iMyPlayOffset = -1;
                            }

                        }

                        ActionManager::GetInstance().AddAction(Action{Cont, ActionType::Play});
                    }
                } else if (request.getURI().find("/playback/play") != std::string::npos) {
                    cRemote::Put(kPlay);
                } else if (request.getURI().find("/playback/pause") != std::string::npos) {
                    cRemote::Put(kPause);
                } else if (request.getURI().find("/playback/stop") != std::string::npos) {
                    cRemote::Put(kStop);
                } else if (request.getURI().find("/playback/stepForward") != std::string::npos) {
                    cHlsPlayerControl *control = dynamic_cast<cHlsPlayerControl *>(cControl::Control(true));
                    if (control) {
                        control->JumpRelative(30);
                    } else if (cMyPlugin::PlayingFile) {
                        cPlugin *mpvPlugin = cPluginManager::GetPlugin("mpv");
                        if (mpvPlugin) {
                            Mpv_Seek seekData;
                            seekData.SeekAbsolute = 0;
                            seekData.SeekRelative = 30;
                            mpvPlugin->Service(MPV_SEEK, &seekData);
                        }
                    } else
                        cRemote::Put(kFastFwd);
                } else if (request.getURI().find("/playback/stepBack") != std::string::npos) {
                    cHlsPlayerControl *control = dynamic_cast<cHlsPlayerControl *>(cControl::Control(true));
                    if (control) {
                        control->JumpRelative(-15);
                    } else if (cMyPlugin::PlayingFile) {
                        cPlugin *mpvPlugin = cPluginManager::GetPlugin("mpv");
                        if (mpvPlugin) {
                            Mpv_Seek seekData;
                            seekData.SeekAbsolute = 0;
                            seekData.SeekRelative = -15;
                            mpvPlugin->Service(MPV_SEEK, &seekData);
                        }
                    } else
                        cRemote::Put(kFastRew);
                } else if (request.getURI().find("/playback/skipNext") != std::string::npos) {
                    cRemote::Put(kFastFwd);
                } else if (request.getURI().find("/playback/skipPrevious") != std::string::npos) {
                    cRemote::Put(kFastRew);
                }

                SubscriptionManager::GetInstance().Notify();
                response.send() << GetOKMsg();
            }

        } catch (Poco::Exception &e) {
            std::cerr << e.displayText() << std::endl;
        }
    }

} // namespace

void plexclient::MirrorRequestHandler::handleRequest(Poco::Net::HTTPServerRequest &request,
                                                     Poco::Net::HTTPServerResponse &response) {
    ///player/mirror/details?type=video&key=%2Flibrary%2Fmetadata%2F113855&machineIdentifier=fbad3115c2c2d82c53b0205e5aa3c4e639ebaa94&protocol=http&address=192.168.1.175&port=32400&token=transient-5557be74-dcad-4c28-badd-aa01b6224862&commandID=2
    UpdateCommandId(request);

    Poco::URI uri(request.getURI());
    std::map<std::string, std::string> query = ParseQuery(uri.getQuery());

    if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_OPTIONS) {
        AddHeaders(response, request);
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK);
        response.send() << " ";
        return;
    }

    if (request.getURI().find("/details") != std::string::npos) {
        std::string protocol = query["protocol"];
        std::string address = query["address"];
        std::string port = query["port"];
        std::string key = query["key"];

        std::string fullUrl = protocol + "://" + address + ":" + port + key; // Metainfo
        auto Cont = Plexservice::GetMediaContainer(fullUrl);
        if(Cont) {
            ActionManager::GetInstance().AddAction(Action {Cont, ActionType::Display});
            AddHeaders(response, request);
        }
        response.send() << GetOKMsg();
    }
}

