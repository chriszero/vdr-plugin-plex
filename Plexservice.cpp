#include "Plexservice.h"

#include "PlexHelper.h"
#include "plexgdm.h"
#include <memory>
#include <Poco/Net/NetException.h>

namespace plexclient {

    Plexservice::Plexservice(PlexServer *server) {
        pServer = server;
    }

    Plexservice::Plexservice(PlexServer *server, std::string startUri) {
        pServer = server;
        StartUri = startUri;
    }

    std::string Plexservice::GetMyPlexToken() {
        static bool done;
        static std::string myToken;

        //todo: cache token in file or db
        if (!done) {
            std::stringstream ss;
            Poco::Base64Encoder b64(ss);

            b64 << Config::GetInstance().GetUsername() << ":" << Config::GetInstance().GetPassword();

            b64.close();
            std::string tempToken = ss.str();


            try {
                Poco::Net::HTTPSClientSession plexSession("plex.tv", 443);
                Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, "/users/sign_in.xml",
                                               Poco::Net::HTTPMessage::HTTP_1_1);

                PlexHelper::AddHttpHeader(request);
                request.add("Authorization", Poco::format("Basic %s", tempToken));

                plexSession.sendRequest(request);

                Poco::Net::HTTPResponse response;
                std::istream &rs = plexSession.receiveResponse(response);
                if (response.getStatus() == 201) {
                    // parse the XML Response
                    user u(&rs);
                    myToken = u.authenticationToken;
                    isyslog("[plex] plex.tv login successfull.");
                } else {
                    esyslog("[plex] plex.tv Login failed, check you creditials.");
                }
                done = true;
                plexSession.abort();
            } catch (Poco::Exception &exc) {
                esyslog("[plex]Exception in %s s%", __func__, exc.displayText().c_str());
                done = true;
            }

        }
        return myToken;
    }

    void Plexservice::Authenticate() {
        if (!Config::GetInstance().UsePlexAccount) return;
        try {
            std::string token = GetMyPlexToken();
            auto pRequest = CreateRequest("/?X-Plex-Token=" + token);

            Poco::Net::HTTPClientSession session(pServer->GetHost(), pServer->GetPort());
            session.sendRequest(*pRequest);
            Poco::Net::HTTPResponse response;
            /*std::istream &rs = */
            session.receiveResponse(response);
            session.abort();
            // TODO: process response
            //Poco::StreamCopier::copyStream(rs, std::cout);
        } catch (Poco::Exception &exc) {
            esyslog("[plex]Exception in %s s%", __func__, exc.displayText().c_str());
        }
    }

    void Plexservice::UpdateResources() {
        // We must be autenticated
        // https://plex.tv/api/resources?includeHttps=1
        if (!Config::GetInstance().UsePlexAccount) {
            isyslog("[plex] To access remote servers, please login with your plex.tv account.");
            return; // Plugin is used without plex.tv login
        }
        isyslog("[plex] Updating remote resources...");


        std::shared_ptr<MediaContainer> pContainer = nullptr;
        try {
            Poco::URI fileuri("https://plex.tv/api/resources?includeHttps=1");

            Poco::Net::HTTPSClientSession session(fileuri.getHost(), 443);

            Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, fileuri.getPathAndQuery(),
                                           Poco::Net::HTTPMessage::HTTP_1_1);
            PlexHelper::AddHttpHeader(request);
            request.add("X-Plex-Token", GetMyPlexToken());

            session.sendRequest(request);
            Poco::Net::HTTPResponse response;
            std::istream &rs = session.receiveResponse(response);

            //Poco::StreamCopier::copyStream(rs, std::cout);

            pContainer = std::shared_ptr<MediaContainer>(new MediaContainer(&rs));

        } catch (Poco::Net::NetException &exc) {
            esyslog("[plex] UpdateResources, NetException: %s", exc.displayText().c_str());
            return;
        } catch (Poco::Exception &exc) {
            esyslog("[plex] UpdateResources, Exception: %s", exc.displayText().c_str());
            return;
        }

        for (std::vector<Device>::iterator d_it = pContainer->m_vDevices.begin();
             d_it != pContainer->m_vDevices.end(); ++d_it) {

            // check device is a server
            if (d_it->m_sProvides.find("server") != std::string::npos) {
                // is it a remote server?
                if (d_it->m_bPublicAddressMatches == false) {
                    // pick remote connection
                    for (std::vector<Connection>::iterator c_it = d_it->m_vConnections.begin();
                         c_it != d_it->m_vConnections.end(); ++c_it) {
                        if (c_it->m_bLocal == false) {
                            dsyslog("[plex] Found server via plex.tv: %s", d_it->m_sName.c_str());
                            // a remote Server
                            plexgdm::GetInstance().AddServer(
                                    PlexServer(c_it->m_sUri, d_it->m_sName, d_it->m_sClientIdentifier,
                                               d_it->m_sAccessToken, d_it->m_bOwned, c_it->m_bLocal));
                        }
                    }
                }
            }
        }
    }

    PlexServer *Plexservice::GetServer() {
        return pServer;
    }

    std::shared_ptr<MediaContainer> Plexservice::GetSection(std::string section, bool putOnStack) {
        try {
            std::string uri;
            if (section[0] == '/') { // Full URI?
                uri = section;
            } else {
                uri = Poco::format("%s/%s", m_vUriStack.top(), section);
            }

            if (putOnStack) {
                m_vUriStack.push(uri);
            }

            dsyslog("[plex] URI: %s%s", pServer->GetUri().c_str(), uri.c_str());

            bool ok;
            auto cSession = pServer->MakeRequest(ok, uri);
            Poco::Net::HTTPResponse response;
            std::istream &rs = cSession->receiveResponse(response);
            if (ok && response.getStatus() == 200) {
                std::shared_ptr<MediaContainer> pAllsections(new MediaContainer(&rs, pServer));
                return pAllsections;
            }
            else {
                dsyslog("[plex] URI: %s%s Response bad: s%", pServer->GetUri().c_str(), uri.c_str(),
                        response.getReasonForStatus(response.getStatus()).c_str());
                return 0;
            }


        } catch (Poco::Net::NetException &exc) {
            pServer->Offline = true;
            return 0;
        }
    }

    std::shared_ptr<MediaContainer> Plexservice::GetLastSection(bool current) {
        if (m_vUriStack.size() > 1) {
            if (!current) {
                // discard last one
                m_vUriStack.pop();
            }
            std::string uri = m_vUriStack.top();
            return GetSection(uri, false);
        }
        return NULL;
    }

    bool Plexservice::IsRoot() {
        return m_vUriStack.size() <= 1;
    }

    std::unique_ptr<Poco::Net::HTTPRequest> Plexservice::CreateRequest(std::string path) {
        std::unique_ptr<Poco::Net::HTTPRequest> pRequest = std::unique_ptr<Poco::Net::HTTPRequest>(
                new Poco::Net::HTTPRequest(Poco::Net::HTTPRequest::HTTP_GET,
                                           path, Poco::Net::HTTPMessage::HTTP_1_1));

        PlexHelper::AddHttpHeader(*pRequest);

        if (Config::GetInstance().UsePlexAccount) {
            // Add PlexToken to Header
            std::string token = GetMyPlexToken();
            if (pServer && !pServer->GetAuthToken().empty()) {
                pRequest->add("X-Plex-Token", pServer->GetAuthToken());
                dsyslog("[plex] Using server access token");
            } else if (!token.empty()) {
                pRequest->add("X-Plex-Token", token);
                dsyslog("[plex] Using global access token");
            }
        }

        return pRequest;
    }

    std::shared_ptr<MediaContainer> Plexservice::GetMediaContainer(std::string fullUrl) {
        PlexServer *pServer = NULL;
        try {
            Poco::URI fileuri(fullUrl);
            dsyslog("[plex] GetMediaContainer: %s", fullUrl.c_str());

            pServer = plexgdm::GetInstance().GetServer(fileuri.getHost(), fileuri.getPort());

            bool ok;
            auto cSession = pServer->MakeRequest(ok, fileuri.getPathAndQuery());
            Poco::Net::HTTPResponse response;
            std::istream &rs = cSession->receiveResponse(response);
            if (ok && response.getStatus() == 200) {
                std::shared_ptr<MediaContainer> pAllsections = std::shared_ptr<MediaContainer>(
                        new MediaContainer(&rs, pServer));
                return pAllsections;
            }
            return 0;
            //Poco::StreamCopier::copyStream(rs, std::cout);
        } catch (Poco::Net::NetException &exc) {
            esyslog("[plex] GetMediaContainer, NetException: %s", exc.displayText().c_str());
            return 0;
        }
    }

    std::string Plexservice::GetUniversalTranscodeUrl(cVideo *video, int offset, PlexServer *server, bool http) {
        PlexServer *pSrv = server ? server : video->m_pServer;
        Poco::URI transcodeUri(pSrv->GetUri());
        if (!http) {
            transcodeUri.setPath("/video/:/transcode/universal/start.m3u8");
            transcodeUri.addQueryParameter("protocol", "hls");
            transcodeUri.addQueryParameter("includeCodecs", "1");
            //transcodeUri.addQueryParameter("copyts", "1");
            transcodeUri.addQueryParameter("directPlay", "0");
            transcodeUri.addQueryParameter("directStream", "1");
            transcodeUri.addQueryParameter("subtitles", "burn");
            transcodeUri.addQueryParameter("audioBoost", "100");
        } else {
            transcodeUri.setScheme("http"); // mpv segfaults with https... :-(
            transcodeUri.setPath("/video/:/transcode/universal/start");
            transcodeUri.addQueryParameter("protocol", "http");

            transcodeUri.addQueryParameter("X-Plex-Client-Identifier", Config::GetInstance().GetUUID());
            transcodeUri.addQueryParameter("X-Plex-Product", "Chromecast");
            transcodeUri.addQueryParameter("X-Plex-Platform", "Chromecast");
            transcodeUri.addQueryParameter("X-Plex-Token", pSrv->GetAuthToken());
        }


        // Force set localhost and http
        Poco::URI pathUri(pSrv->GetUri() + video->m_sKey);
        pathUri.setHost("127.0.0.1");
        pathUri.setScheme("http");

        transcodeUri.addQueryParameter("path", pathUri.toString());
        transcodeUri.addQueryParameter("mediaIndex", "0");
        transcodeUri.addQueryParameter("partIndex", "0");
        transcodeUri.addQueryParameter("offset", std::to_string(offset));
        transcodeUri.addQueryParameter("fastSeek", "1");


        if (pSrv->IsLocal()) {
            transcodeUri.addQueryParameter("videoResolution", "1920x1080");
            transcodeUri.addQueryParameter("maxVideoBitrate", "20000");
            transcodeUri.addQueryParameter("videoQuality", "100");
        } else {
            transcodeUri.addQueryParameter("videoResolution", "1280x720");
            transcodeUri.addQueryParameter("maxVideoBitrate", "8000");
            transcodeUri.addQueryParameter("videoQuality", "100");
        }
        transcodeUri.addQueryParameter("session", Config::GetInstance().GetUUID()); // TODO: generate Random SessionID


        if (Config::GetInstance().UseAc3 && !http) {
            transcodeUri.addQueryParameter("X-Plex-Client-Profile-Extra",
                                           "add-transcode-target-audio-codec(type=videoProfile&context=streaming&protocol=hls&audioCodec=ac3)");
            //params << encode("+add-limitation(scope=videoCodec&scopeName=h264&type=lowerBound&name=video.height&value=1080)");
            //params << encode("+add-limitation(scope=videoCodec&scopeName=h264&type=lowerBound&name=video.frameRate&value=25)");
            //params << encode("+add-limitation(scope=videoCodec&scopeName=h264&type=upperBound&name=video.frameRate&value=25)");
        }

        return transcodeUri.toString();
    }

}
