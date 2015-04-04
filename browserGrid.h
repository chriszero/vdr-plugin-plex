#ifndef CBROWSERGRID_H
#define CBROWSERGRID_H

#include <memory>
#include <vector>
#include <functional>
#include "Plexservice.h"
#include "plexgdm.h"
#include "PlexServer.h"
#include "viewGridNavigator.h"
#include "libskindesigner/osdelements.h"

class cDummyElement : public cGridElement
{
public:
	virtual std::string GetTitle();
	virtual void AddTokens(std::shared_ptr<cOsdElement> grid, bool clear = true, std::function<void(cGridElement*)> OnCached = NULL);
};

class cServerElement : public cGridElement
{
private:
	plexclient::PlexServer* m_pServer;
	std::string m_sStartPath;
	std::string m_sStartName;
public:
	cServerElement(plexclient::PlexServer* server, std::string startPath, std::string startName);
	virtual std::string GetTitle();
	virtual void AddTokens(std::shared_ptr<cOsdElement> grid, bool clear = true, std::function<void(cGridElement*)> OnCached = NULL);
	std::string StartPath() { return m_sStartPath; }
	plexclient::PlexServer* Server() { return m_pServer; }
};

class cBrowserGrid : public cViewGridNavigator
{
private:
	bool m_bServersAreRoot;
	std::vector<cServerElement> m_vServerElements;
	std::shared_ptr<plexclient::MediaContainer> m_pContainer;
	std::shared_ptr<plexclient::Plexservice> m_pService;
	cDummyElement m_Dummy;

	void ProcessData();
	void SetServerElements();
	
public:
	cBrowserGrid(cViewGrid* viewGrid);
	cBrowserGrid(cViewGrid* viewGrid, std::shared_ptr<plexclient::Plexservice> service);
	std::shared_ptr<plexclient::MediaContainer> MediaContainer() { return m_pContainer; }
		
	virtual eOSState NavigateSelect();
	virtual eOSState NavigateBack();
};

#endif // CBROWSERGRID_H
