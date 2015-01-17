#ifndef MEDIA_H
#define MEDIA_H

#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/NamedNodeMap.h>
#include <Poco/DOM/NodeIterator.h>
#include <Poco/DOM/NodeFilter.h>
#include <Poco/DOM/NodeList.h>
#include <Poco/DOM/AutoPtr.h>
#include <Poco/Exception.h>
#include <Poco/Timestamp.h>
#include <Poco/String.h>

#include <vector>
#include <iostream>

#include "XmlObject.h" // Base class: model::XmlObject

using Poco::XML::DOMParser;
using Poco::XML::Document;
using Poco::XML::NodeIterator;
using Poco::XML::NodeFilter;
using Poco::XML::Node;
using Poco::XML::AutoPtr;
using Poco::Exception;

namespace plexclient
{

class Media: XmlObject
{
public:
	Media(Poco::XML::Node* pNode);
	~Media();

public:
	std::string m_sVideoResolution;
	int m_iId;
	long m_lDuration;
	int m_iBitrate;
	int m_iWidth;
	int m_iHeight;
	std::string m_sAspectRatio;
	int m_iAudioChannels;
	std::string m_sAudioCodec;
	std::string m_sVideoCodec;
	std::string m_sContainer;
	double m_VideoFrameRate;

	std::string m_sPartKey;
	int m_iPartId;
	long m_lPartDuration;
	std::string m_sPartFile;
	long m_lPartSize;
	std::string m_sPartContainer;

};

}

#endif // MEDIA_H
