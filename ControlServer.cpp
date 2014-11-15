#include "ControlServer.h"

namespace plexclient
{

void ControlServer::Start() {
	// start the HTTPServer
	m_pSrv->start();
}

void ControlServer::Stop() {
	// Stop the HTTPServer
	m_pSrv->stop();
}


}
