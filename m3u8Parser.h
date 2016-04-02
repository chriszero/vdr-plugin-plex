#ifndef M3U8PARSER_H
#define M3U8PARSER_H

#include <vector>
#include <string>
#include <iostream>

class cM3u8Parser {
public:
    struct playListItem {
        int length;
        int bandwidth;
        int programId;
        std::string file;
        int size;
    };
private:
    void Init();

    void eDump(std::istream &m3u8);

public:
    std::vector<playListItem> vPlaylistItems;
    int TargetDuration;
    int MediaSequence;
    bool MasterPlaylist;
    bool AllowCache;

    bool Parse(std::istream &m3u8);

public:
    cM3u8Parser(std::istream &m3u8);

    cM3u8Parser();

    ~cM3u8Parser() { };

};


#endif // M3U8PARSER_H
