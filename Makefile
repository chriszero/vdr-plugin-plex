#
# Makefile for a Video Disk Recorder plugin
#
# $Id: ac85bc689caa43a6c25d7e0573733b36fab6167e $

# The official name of this plugin.
# This name will be used in the '-P...' option of VDR to load the plugin.
# By default the main source file also carries this name.

PLUGIN = plex
VERSION := $(shell git describe --tags master)

LIBS += -lPocoUtil -lPocoNet -lPocoNetSSL -lPocoXML -lPocoFoundation -lpcrecpp
LIBS += $(shell pkg-config --libs libskindesignerapi)

### Configuration (edit this for your needs)

#CONFIG := -DDEBUG			# uncomment to build DEBUG
DISABLESKINDESIGNER ?= 0

### The version number of this plugin (taken from the main source file):

VERSION = $(shell grep 'static const char \*const VERSION *=' $(PLUGIN).h | awk '{ print $$7 }' | sed -e 's/[";]//g')
GIT_REV = $(shell git describe --tags --always 2>/dev/null)

### The directory environment:

# Use package data if installed...otherwise assume we're under the VDR source directory:
PKGCFG = $(if $(VDRDIR),$(shell pkg-config --variable=$(1) $(VDRDIR)/vdr.pc),$(shell pkg-config --variable=$(1) vdr || pkg-config --variable=$(1) ../../../vdr.pc))
LIBDIR = $(call PKGCFG,libdir)
LOCDIR = $(call PKGCFG,locdir)
PLGCFG = $(call PKGCFG,plgcfg)
PLGRESDIR = $(call PKGCFG,resdir)/plugins/skindesigner
#
TMPDIR ?= /tmp

### The compiler options:

export CFLAGS	= $(call PKGCFG,cflags)
export CXXFLAGS = $(call PKGCFG,cxxflags)

#CXXFLAGS += -std=gnu++0x
export CXXFLAGS += -std=c++11

### The version number of VDR's plugin API:

APIVERSION = $(call PKGCFG,apiversion)

### Allow user defined options to overwrite defaults:

-include $(PLGCFG)

### The name of the distribution archive:

ARCHIVE = $(PLUGIN)-$(VERSION)
PACKAGE = vdr-$(ARCHIVE)

### The name of the shared object file:

SOFILE = libvdr-$(PLUGIN).so

### Includes and Defines (add further entries here):
ifneq ($(DISABLESKINDESIGNER),1)
CONFIG += -DSKINDESIGNER
INCLUDES += $(shell pkg-config --cflags libskindesignerapi)

DEFINES += -DLIBSKINDESIGNERAPIVERSION='"$(shell pkg-config --modversion libskindesignerapi)"'
endif

DEFINES += -DPLUGIN_NAME_I18N='"$(PLUGIN)"' -DPLUGIN='"$(PLUGIN)"' -D_GNU_SOURCE $(CONFIG) \
	$(if $(GIT_REV), -DGIT_REV='"$(GIT_REV)"')

### The object files (add further files here):

OBJS = $(PLUGIN).o \
	Config.o \
	ControlServer.o \
	cPlexOsdItem.o \
	m3u8Parser.o \
	hlsPlayer.o \
	hlsPlayerControl.o \
	plexgdm.o \
	PlexHelper.o \
	Plexservice.o \
	PlexServer.o \
	PlexHTTPRequestHandler.o \
	PlexReqHandlerFactory.o \
	SubscriptionManager.o \
	user.o \
	XmlObject.o \
	MediaContainer.o \
	Directory.o \
	PVideo.o \
	Stream.o \
	Media.o \
	plexOsd.o \
	device.o \
	playlist.o

ifneq ($(DISABLESKINDESIGNER),1)
OBJS += plexSdOsd.o \
	viewGridNavigator.o \
	browserGrid.o \
	viewHeader.o \
	detailView.o \
	pictureCache.o \
	displayReplaySD.o \
	sdGenericViewElements.o
endif

SRCS = $(wildcard $(OBJS:.o=.c)) $(PLUGIN).cpp

### The main target:

all: $(SOFILE) i18n

### Implicit rules:

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) -o $@ $<

### Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(CXXFLAGS) $(SRCS) > $@

-include $(DEPFILE)

### Internationalization (I18N):

PODIR	  = po
I18Npo	  = $(wildcard $(PODIR)/*.po)
I18Nmo	  = $(addsuffix .mo, $(foreach file, $(I18Npo), $(basename $(file))))
I18Nmsgs  = $(addprefix $(DESTDIR)$(LOCDIR)/, $(addsuffix /LC_MESSAGES/vdr-$(PLUGIN).mo, $(notdir $(foreach file, $(I18Npo), $(basename $(file))))))
I18Npot	  = $(PODIR)/$(PLUGIN).pot

%.mo: %.po
	msgfmt -c -o $@ $<

$(I18Npot): $(wildcard *.cpp)
	xgettext -C -cTRANSLATORS --no-wrap --no-location -k -ktr -ktrNOOP --package-name=vdr-$(PLUGIN) --package-version=$(VERSION) --msgid-bugs-address='<see README>' -o $@ `ls $^`

%.po: $(I18Npot)
	msgmerge -U --no-wrap --no-location --backup=none -q -N $@ $<
	@touch $@

$(I18Nmsgs): $(DESTDIR)$(LOCDIR)/%/LC_MESSAGES/vdr-$(PLUGIN).mo: $(PODIR)/%.mo
	install -D -m644 $< $@

.PHONY: i18n
i18n: $(I18Nmo) $(I18Npot)

install-i18n: $(I18Nmsgs)

### Targets:

$(OBJS): Makefile

$(SOFILE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared $(OBJS) $(LIBS) -o $@

install-lib: $(SOFILE)
	install -D $^ $(DESTDIR)$(LIBDIR)/$^.$(APIVERSION)

install-skins:
	mkdir -p $(DESTDIR)$(PLGRESDIR)/skins
	cp -r skins/* $(DESTDIR)$(PLGRESDIR)/skins

install: install-lib install-i18n

dist: $(I18Npo) clean
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@mkdir $(TMPDIR)/$(ARCHIVE)
	@cp -a * $(TMPDIR)/$(ARCHIVE)
	@tar czf $(PACKAGE).tgz -C $(TMPDIR) $(ARCHIVE)
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@echo Distribution package created as $(PACKAGE).tgz

clean:
	@-rm -f $(PODIR)/*.mo $(PODIR)/*.pot
	@-rm -f $(OBJS) $(DEPFILE) *.so *.tgz core* *~

## Private Targets:

HDRS=	$(wildcard *.h)

indent:
	for i in $(SRCS) $(HDRS); do \
		indent $$i; \
		unexpand -a $$i | sed -e s/constconst/const/ > $$i.up; \
		mv $$i.up $$i; \
	done
