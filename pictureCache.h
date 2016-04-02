#ifndef CPICTURECACHE_H
#define CPICTURECACHE_H

#include <memory>
#include <iostream>
#include <fstream>
#include <deque>
#include <string>
#include <future>

#include <vdr/thread.h>
#include <vdr/plugin.h>

#include "viewGridNavigator.h"

enum ImageResolution {
    SD384,
    SD480,
    HD720,
    HD1080
};

class cPictureCache {
private:
    struct CacheInfo {
        CacheInfo(std::string Uri, int Width, int Height, std::function<void(cGridElement *)> OnCached,
                  cGridElement *Calle) {
            uri = Uri;
            width = Width;
            height = Height;
            onCached = OnCached;
            calle = Calle;
        };
        std::string uri;
        int width;
        int height;
        std::function<void(cGridElement *)> onCached;
        cGridElement *calle;
        bool downloaded;
    };

    cPictureCache();

    std::map<std::string, bool> m_mCached;
    std::vector<std::future<void>> m_vFutures;
    volatile bool m_bAllInvalidated;

    std::string FileName(std::string uri, int width);

    std::string TranscodeUri(std::string uri, int width, int height);

    static bool DownloadFileAndSave(std::string uri, std::string localFile);

    bool Cached(std::string uri, int width);

    std::string m_cacheDir;

public:
    static cPictureCache &GetInstance() {
        static cPictureCache instance;
        return instance;
    }

    std::string GetPath(std::string uri, int width, int height, bool &cached,
                        std::function<void(cGridElement *)> OnCached = NULL, cGridElement *calle = NULL);
};

#endif // CPICTURECACHE_H
