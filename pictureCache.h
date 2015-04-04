#ifndef CPICTURECACHE_H
#define CPICTURECACHE_H

#include <memory>
#include <iostream>
#include <fstream>
#include <deque>
#include <string>

#include <vdr/thread.h>
#include <vdr/plugin.h>

#include "viewGridNavigator.h"

enum ImageResolution {
	SD384,
	SD480,
	HD720,
	HD1080
};

class cPictureCache : public cThread
{
private:
	struct CacheInfo {
		CacheInfo(std::string Uri, int Width, int Height, std::function<void(cGridElement*)> OnCached, cGridElement* Calle) {
			uri = Uri;
			width = Width;
			height = Height;
			onCached = OnCached;
			calle = Calle;
		};
		std::string uri;
		int width;
		int height;
		std::function<void(cGridElement*)> onCached;
		cGridElement* calle;
	};
	cPictureCache();
	std::deque<CacheInfo> m_qImagesToLoad;


	std::string FileName(std::string uri, int width);

	std::shared_ptr<std::istream> DownloadFile(std::string uri);
	void SaveFileToDisk(std::shared_ptr<std::istream> file, std::string fileName);
	std::string TranscodeUri(std::string uri, int width, int height);

	std::string m_cacheDir;

protected:
	virtual void Action();

public:
	static cPictureCache& GetInstance() {
		static cPictureCache instance;
		return instance;
	}
	void Stop();

	bool Cached(std::string uri, int width);
	std::string GetPath(std::string uri, int width, int height, bool& cached, std::function<void(cGridElement*)> OnCached = NULL, cGridElement* calle = NULL);
	void Remove(cGridElement* element);
	void Remove(std::string uri);
};

#endif // CPICTURECACHE_H
