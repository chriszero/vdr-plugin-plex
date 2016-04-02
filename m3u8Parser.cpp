#include "m3u8Parser.h"
#include <pcrecpp.h>
#include <vdr/tools.h>

cM3u8Parser::cM3u8Parser() {
    Init();
}

cM3u8Parser::cM3u8Parser(std::istream &m3u8) {
    Init();
    Parse(m3u8);
}

void cM3u8Parser::Init() {
    TargetDuration = -1;
    MediaSequence = -1;
    MasterPlaylist = false;
    AllowCache = true;
}


bool cM3u8Parser::Parse(std::istream &m3u8) {
    bool ok = true;
    bool nextLineIsMedia = false;
    int lineNo = 0;

    // prepare regex
    pcrecpp::RE re("#(EXT[^:\\n]+)(?::[^\\n]+)");
    pcrecpp::RE reVal("(?::([^\\n]+))");

    std::string line;
    playListItem pItem;
    pItem.bandwidth = -1;
    pItem.file = "";
    pItem.length = -1;
    pItem.programId = -1;

    while (std::getline(m3u8, line)) {
        if (lineNo == 0 && "#EXTM3U" == line) {
            lineNo++;
            continue;
        } else if (lineNo == 0) {
            // Invalid File
            ok = false;
            esyslog("[plex]%s m3u8 is invalid. dumping File:", __FUNCTION__);
            esyslog("[plex]%s", line.c_str());
            eDump(m3u8);
            break;
        }

        if (re.FullMatch(line)) {
            string var;
            //string value;
            re.PartialMatch(line, &var);
            if ("EXT-X-TARGETDURATION" == var) {
                int value;
                reVal.PartialMatch(line, &value);
                TargetDuration = value;
            } else if ("EXT-X-ALLOW-CACHE" == var) {
                string value;
                reVal.PartialMatch(line, &value);
                AllowCache = "YES" == value;
            } else if ("EXT-X-MEDIA-SEQUENCE" == var) {
                int value;
                reVal.PartialMatch(line, &value);
                MediaSequence = value;
            } else if ("EXT-X-STREAM-INF" == var) {
                MasterPlaylist = true;
                nextLineIsMedia = true;

                int bandw;
                pcrecpp::RE reBand("BANDWIDTH=(\\d+)");
                if (reBand.PartialMatch(line, &bandw)) {
                    pItem.bandwidth = bandw;
                }

                int id;
                pcrecpp::RE reId("PROGRAM-ID=(\\d+)");
                if (reId.PartialMatch(line, &id)) {
                    pItem.programId = id;
                }

            } else if ("EXTINF" == var) {
                MasterPlaylist = false;
                int value;
                pcrecpp::RE reInt("(?:#EXTINF:([\\d]+))");
                if (reInt.PartialMatch(line, &value)) {
                    nextLineIsMedia = true;
                    pItem.length = value;
                }
            }
        }
            // possible mediafile
        else {
            if (nextLineIsMedia) {
                nextLineIsMedia = false;
                pItem.file = line;
                pItem.size = 0;
                vPlaylistItems.push_back(pItem);
            }
        }
        lineNo++;
    }
    return ok;
}

void cM3u8Parser::eDump(std::istream &m3u8) {
    std::string line;
    while (std::getline(m3u8, line)) {
        esyslog("[plex]%s", line.c_str());
    }
}

/*
#EXTM3U
#EXT-X-STREAM-INF:PROGRAM-ID=1,BANDWIDTH=4000000
session/731d4143-c05d-47ed-afa8-37913402630a/8/index.m3u8
*/

/*
#EXTM3U
#EXT-X-TARGETDURATION:3
#EXT-X-ALLOW-CACHE:NO
#EXT-X-MEDIA-SEQUENCE:0
#EXTINF:3, nodesc
00000.ts
#EXTINF:3, nodesc
00001.ts
...
#EXTINF:2, nodesc
01935.ts
#EXT-X-ENDLIST
*/
