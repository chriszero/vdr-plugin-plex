#include "ControlServer.h"

namespace plexclient
{

ControlServer::ControlServer() {
	m_pSvs = new Poco::Net::ServerSocket(3200);;
	m_pSrv = new Poco::Net::HTTPServer(new PlexReqHandlerFactory, *m_pSvs, new Poco::Net::HTTPServerParams);
}
	
void ControlServer::Start() {
	// start the HTTPServer
	m_pSrv->start();
}

void ControlServer::Stop() {
	// Stop the HTTPServer
	m_pSrv->stop();
}


}
