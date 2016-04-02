#include "PlexHelper.h"
#include "Config.h"
#include "plex.h"

namespace plexclient {

    void PlexHelper::AddHttpHeader(Poco::Net::HTTPRequest &request) {
        request.add("User-Agent",
                    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_8_2) AppleWebKit/537.17 (KHTML, like Gecko) Chrome/24.0.1312.52 Safari/537.17");

        //request.add("X-Plex-Client-Capabilities", "protocols=shoutcast,http-video;videoDecoders=h264{profile:high&resolution:1080&level:51};audioDecoders=mp3,aac");
        request.add("X-Plex-Client-Identifier", Config::GetInstance().GetUUID());
        request.add("X-Plex-Device", "PC");
        request.add("X-Plex-Device-Name", Config::GetInstance().GetHostname());
        request.add("X-Plex-Language", Config::GetInstance().GetLanguage());
        request.add("X-Plex-Model", "Linux");
        request.add("X-Plex-Platform", "VDR");
        request.add("X-Plex-Product", "plex for vdr");
        request.add("X-Plex-Provides", "player");
        request.add("X-Plex-Version", VERSION);
    }


} // namespace
