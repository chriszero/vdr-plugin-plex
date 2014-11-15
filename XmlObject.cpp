#include "XmlObject.h"

namespace plexclient
{

XmlObject::XmlObject()
{
}

XmlObject::~XmlObject()
{
}

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
	int value = 0;
	if(pNode != 0) {
		try {
			value = std::stoi(pNode->getNodeValue());
		} catch(Poco::Exception) {}
	}
	return value;
}

long XmlObject::GetNodeValueAsLong(Poco::XML::Node* pNode)
{
	long value = 0;
	if(pNode != 0) {
		try {
			value = std::stol(pNode->getNodeValue());
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
			long lValue = std::stol(pNode->nodeValue());
			value = Poco::Timestamp(lValue);
		} catch (Poco::Exception) {}
	}
	return value;
}

MediaType XmlObject::GetNodeValueAsMediaType(Poco::XML::Node* pNode)
{
	MediaType type = MediaType::UNDEF;

	if(pNode != 0) {
		std::string sType = pNode->nodeValue();
		if (Poco::icompare(sType, "photo") == 0) {
			type = MediaType::PHOTO;
		} else if (Poco::icompare(sType, "movie") == 0) {
			type = MediaType::MOVIE;
		} else if (Poco::icompare(sType, "music") == 0) {
			type = MediaType::MUSIC;
		} else if (Poco::icompare(sType, "show") == 0) {
			type = MediaType::SHOW;
		} else if (Poco::icompare(sType, "season") == 0) {
			type = MediaType::SHOW;
		}
	}
	return type;
}

}
