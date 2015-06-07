#include "XmlObject.h"
#include <vdr/tools.h>

namespace plexclient
{
std::string XmlObject::GetNodeValue(Poco::XML::Node* pNode)
{
	std::string value;
	if(pNode != 0) {
		value = pNode->getNodeValue();
	}
	return value;
}

int XmlObject::GetNodeValueAsInt(Poco::XML::Node* pNode)
{
	int value = -1;
	if(pNode != 0) {
		try {
			value = atoi(pNode->getNodeValue().c_str());
		} catch(Poco::Exception) {}
	}
	return value;
}

long XmlObject::GetNodeValueAsLong(Poco::XML::Node* pNode)
{
	long value = -1;
	if(pNode != 0) {
		try {
			value = atol(pNode->getNodeValue().c_str());
		} catch(Poco::Exception) {}
	}
	return value;
}

double XmlObject::GetNodeValueAsDouble(Poco::XML::Node* pNode)
{
	double value = -1;
	if(pNode != 0) {
		try {
			value = atod(pNode->getNodeValue().c_str());
		} catch(Poco::Exception) {}
	}
	return value;
}

bool XmlObject::GetNodeValueAsBool(Poco::XML::Node* pNode)
{
	bool value = false;
	if(pNode != 0) {
		value = pNode->getNodeValue() == "1";
	}
	return value;
}

Poco::Timestamp XmlObject::GetNodeValueAsTimeStamp(Poco::XML::Node* pNode)
{
	Poco::Timestamp value;
	if(pNode != 0) {
		try {
			long lValue = atol(pNode->nodeValue().c_str());
			value = Poco::Timestamp(lValue);
		} catch (Poco::Exception) {}
	}
	return value;
}

MediaType XmlObject::GetNodeValueAsMediaType(Poco::XML::Node* pNode)
{
	MediaType type = UNDEF;

	if(pNode != 0) {
		std::string sType = pNode->nodeValue();
		if (Poco::icompare(sType, "photo") == 0) {
			type = PHOTO;
		} else if (Poco::icompare(sType, "movie") == 0) {
			type = MOVIE;
		} else if (Poco::icompare(sType, "music") == 0) {
			type = MUSIC;
		} else if (Poco::icompare(sType, "show") == 0) {
			type = SHOW;
		} else if (Poco::icompare(sType, "season") == 0) {
			type = SEASON;
		} else if (Poco::icompare(sType, "episode") == 0) {
			type = EPISODE;
		} else if (Poco::icompare(sType, "clip") == 0) {
			type = CLIP;
		}
	}
	return type;
}

StreamType XmlObject::GetNodeValueAsStreamType(Poco::XML::Node* pNode)
{
	StreamType type = sUNDEF;

	if(pNode != 0) {
		int iType = GetNodeValueAsInt(pNode);
		switch(iType) {
			case 1: type = sVIDEO; break;
			case 2: type = sAUDIO; break;
			case 3: type = sSUBTITLE; break;
			default: type = sUNDEF; break;
		}
	}
	return type;
}

}
