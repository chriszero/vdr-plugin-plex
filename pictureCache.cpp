#include "pictureCache.h"
#include <Poco/Format.h>
#include <Poco/URIStreamOpener.h>
#include <Poco/StreamCopier.h>
#include <Poco/Path.h>
#include <Poco/File.h>
#include <Poco/StringTokenizer.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>
#include "plexSdOsd.h"
#include "PVideo.h"
#include "Directory.h"
#include "plexgdm.h"
#include "PlexServer.h"

using Poco::URIStreamOpener;
using Poco::StreamCopier;
using Poco::Path;
using Poco::Exception;

cPictureCache::cPictureCache() {
    m_cacheDir = cPlugin::CacheDirectory(PLUGIN_NAME_I18N);

    Poco::Path path(m_cacheDir);
    Poco::File f(path.toString());
    f.createDirectories();
    m_bAllInvalidated = false;
}

bool cPictureCache::DownloadFileAndSave(std::string Uri, std::string localFile) {
    try {
        Poco::URI fileUri(Uri);
        plexclient::PlexServer *pServer = plexclient::plexgdm::GetInstance().GetServer(fileUri.getHost(),
                                                                                       fileUri.getPort());

        bool ok;
        auto cSession = pServer->MakeRequest(ok, fileUri.getPathAndQuery());
        Poco::Net::HTTPResponse response;
        std::istream &rs = cSession->receiveResponse(response);

        if (!ok || response.getStatus() != 200)
            return false;

        std::string type = response.getContentType();
        std::string fileName = localFile;

        // check filetype
        char buffer[2];
        rs.read(buffer, 2);
        bool isPng = false;

        if (buffer[0] == char(0x89) && buffer[1] == char(0x50)) {
            isPng = true;
        }
        rs.putback(buffer[1]);
        rs.putback(buffer[0]);

        if (isPng) {
            fileName += ".png";
        } else if (type == "image/jpeg" || fileUri.getPathAndQuery().find(".jpg") != std::string::npos) {
            fileName += ".jpg";
        }

        Poco::Path p(fileName);
        Poco::File f(p.makeParent().toString());
        f.createDirectories();

        std::ofstream outFile;
        outFile.open(fileName);
        Poco::StreamCopier::copyStream(rs, outFile);
        outFile.close();
        return true;
    } catch (Poco::Exception &exc) {
        std::cout << exc.displayText() << std::endl;
        return false;
    }
}

std::string cPictureCache::FileName(std::string uri, int width) {
    Poco::URI u(uri);
    plexclient::PlexServer *pServer = plexclient::plexgdm::GetInstance().GetServer(u.getHost(), u.getPort());
    std::string uuid = "default";
    if (pServer && !pServer->GetUuid().empty()) {
        uuid = pServer->GetUuid();
    }
    std::string file = Poco::format("%s/%s_%d", uuid, u.getPathAndQuery(), width);

    Poco::Path path(m_cacheDir);
    path.append(file);

    return path.toString();
}

std::string cPictureCache::TranscodeUri(std::string uri, int width, int height) {
    // We have to transform the uri a little...
    // httpX://ServerUri/.../queryUri=localhost

    Poco::URI u(uri);
    auto plServer = plexclient::plexgdm::GetInstance().GetServer(u.getHost(), u.getPort());
    u.setHost("127.0.0.1"); // set to localhost and http only!
    u.setScheme("http");

    Poco::URI serverUri(plServer->GetUri());
    serverUri.setPath("/photo/:/transcode");
    serverUri.addQueryParameter("width", std::to_string(width));
    serverUri.addQueryParameter("height", std::to_string(height));
    serverUri.addQueryParameter("url", u.toString());

    return serverUri.toString();
}

bool cPictureCache::Cached(std::string uri, int width) {
    bool cached = true;
    try {
        cached = m_mCached.at(FileName(uri, width));
    } catch (std::out_of_range) { }

    bool onDisk =
            Poco::File(FileName(uri, width) + ".jpg").exists() || Poco::File(FileName(uri, width) + ".png").exists();
    return onDisk && cached;
}

std::string cPictureCache::GetPath(std::string uri, int width, int height, bool &cached,
                                   std::function<void(cGridElement *)> OnCached, cGridElement *calle) {
    m_bAllInvalidated = false;
    cached = Cached(uri, width);
    std::string file = FileName(uri, width);
    if (cached) {
        if (Poco::File(FileName(uri, width) + ".jpg").exists()) {
            file += ".jpg";
        } else if (Poco::File(FileName(uri, width) + ".png").exists()) {
            file += ".png";
        }
        return file;
    } else {

        try {
            m_mCached.at(file);
            return file;
        } catch (std::out_of_range) { }

        std::string transcodeUri = TranscodeUri(uri, width, height);

        m_mCached[file] = false;

        m_vFutures.push_back(std::async(std::launch::async,
                                        [&](std::string tsUri, std::string fn,
                                            std::function<void(cGridElement *)> onCached, cGridElement *ca) {
                                            bool ok = DownloadFileAndSave(tsUri, fn);
                                            if (ok) {
                                                m_mCached[fn] = true;
                                            }
                                            if (ok && onCached && ca && ca->IsVisible()) {
                                                onCached(ca);
                                            }
                                            return;
                                        },
                                        transcodeUri, file, OnCached, calle));
    }

    return file;
}


