#include "pictureCache.h"
#include <Poco/Format.h>
#include <Poco/URIStreamOpener.h>
#include <Poco/StreamCopier.h>
#include <Poco/Path.h>
#include <Poco/File.h>
#include <Poco/StringTokenizer.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>
#include <Poco/Net/HTTPStreamFactory.h>
#include "plexSdOsd.h"
#include "PVideo.h"
#include "Directory.h"

using Poco::URIStreamOpener;
using Poco::StreamCopier;
using Poco::Path;
using Poco::Exception;
using Poco::Net::HTTPStreamFactory;

cPictureCache::cPictureCache()
{
	m_cacheDir = cPlugin::CacheDirectory(PLUGIN_NAME_I18N);

	Poco::Path path(m_cacheDir);
	Poco::File f(path.toString());
	f.createDirectories();
	HTTPStreamFactory::registerFactory();
}

void cPictureCache::Action()
{
	while(Running()) {
		while (m_qImagesToLoad.size() > 0) {
			//LOCK_THREAD;
			CacheInfo info = m_qImagesToLoad.front();
			m_qImagesToLoad.pop_front();

			std::string transcodeUri = TranscodeUri(info.uri, info.width, info.height);
			std::string file = FileName(info.uri, info.width);
			if(!Cached(info.uri, info.width)) {
				auto stream = DownloadFile(transcodeUri);
				if(stream) {
					SaveFileToDisk(stream, file);
					cMutexLock MutexLock(&cPlexSdOsd::RedrawMutex);
					if (info.onCached && info.calle && info.calle->IsVisible()) { 
						info.onCached(info.calle);
					}
				}
			}
			cCondWait::SleepMs(5);
		}
		cCondWait::SleepMs(100);
	}
}

std::shared_ptr<std::istream> cPictureCache::DownloadFile(std::string uri)
{
	try {
		std::shared_ptr<std::istream> pStream(URIStreamOpener::defaultOpener().open(uri));
		return pStream;
	} catch (Poco::Exception &exc) {
		return NULL;
	}
}

void cPictureCache::SaveFileToDisk(std::shared_ptr<std::istream> file, std::string fileName)
{
	try {
		Poco::Path p(fileName);
		Poco::File f(p.makeParent().toString());
		f.createDirectories();
		
		std::ofstream outFile;
		outFile.open(fileName);
		Poco::StreamCopier::copyStream(*file, outFile);
		outFile.close();
	} catch (Poco::Exception &exc) {
		std::cout << "SaveFile Error: " << exc.displayText() << std::endl;
	}
}

std::string cPictureCache::FileName(std::string uri, int width)
{
	Poco::URI u(uri);
	std::string file = Poco::format("%s_%d", u.getPath(), width);

	Poco::Path path(m_cacheDir);
	path.append(file);

	return path.toString();
}

std::string cPictureCache::TranscodeUri(std::string uri, int width, int height)
{
	std::string escapedUri;
	Poco::URI::encode(uri, " !\"#$%&'()*+,/:;=?@[]", escapedUri);
	Poco::URI u(uri);
	int port = u.getPort();
	std::string tUri = Poco::format("http://%s:%d/photo/:/transcode?width=%d&height=%d&url=%s", u.getHost(), port, width, height, escapedUri);
	return tUri;
}

bool cPictureCache::Cached(std::string uri, int width)
{
	return Poco::File(FileName(uri, width)).exists();
}

std::string cPictureCache::GetPath(std::string uri, int width, int height, bool& cached, std::function<void(cGridElement*)> OnCached, cGridElement* calle)
{
	cached = Cached(uri, width);
	std::string file = FileName(uri, width);
	if(cached) {
		return file;
	} else {
		//LOCK_THREAD;
		CacheInfo info(uri, width, height, OnCached, calle);
		m_qImagesToLoad.push_back(info);
	}

	return file;
}

void cPictureCache::Stop()
{
	Cancel();
}

void cPictureCache::Remove(cGridElement* element)
{
	if(!element) return;
	if (auto video = dynamic_cast<plexclient::Video*>(element)) {
		Remove(video->ThumbUri());
		Remove(video->ArtUri());
	}
}

void cPictureCache::Remove(std::string uri)
{
	for(std::deque<CacheInfo>::iterator it = m_qImagesToLoad.begin() ; it != m_qImagesToLoad.end(); ++it) {
		if(it->uri == uri) {
			m_qImagesToLoad.erase(it);
			return;
		}
	}
}
