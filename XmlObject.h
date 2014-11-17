#ifndef XMLOBJECT_H
#define XMLOBJECT_H

#include <cstdlib>

#include <Poco/DOM/Document.h>
#include <Poco/Timestamp.h>
#include <Poco/String.h>
#include <Poco/Exception.h>

namespace plexclient
{

enum MediaType {UNDEF = 0, PHOTO, MOVIE, MUSIC, SHOW, SEASON};

class XmlObject
{
public:
	XmlObject();
	~XmlObject();

protected:
	std::string GetNodeValue(Poco::XML::Node* pNode);
	int GetNodeValueAsInt(Poco::XML::Node* pNode);
	long GetNodeValueAsLong(Poco::XML::Node* pNode);
	bool GetNodeValueAsBool(Poco::XML::Node* pNode);
	Poco::Timestamp GetNodeValueAsTimeStamp(Poco::XML::Node* pNode);
	MediaType GetNodeValueAsMediaType(Poco::XML::Node* pNode);

private:

};

}

#endif // XMLOBJECT_H
