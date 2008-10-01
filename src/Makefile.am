
AM_CXXFLAGS = -Wall -Werror -Wextra -Wno-unused-parameter -Wpointer-arith -Wcast-align -pedantic -ansi -std=c++98 ${CFLAGS}

localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

bin_PROGRAMS = nightfall
nightfall_SOURCES = main.cpp window.cpp game.cpp utilities.cpp ogrexmlmodel.cpp audio.cpp terrain.cpp \
                    aibase.cpp dimension.cpp font.cpp unit.cpp aipathfinding.cpp vector3d.cpp luawrapper.cpp \
                    unitinterface.cpp gui.cpp environment.cpp console.cpp textures.cpp \
                    effect.cpp camera.cpp gamegui.cpp randomgenerator.cpp networking.cpp filesystem.cpp \
                    paths.cpp circularbuffer.cpp minixml.cpp saveandload.cpp unittype.cpp unitsquares.cpp \
                    unitrender.cpp scenegraph.cpp matrix4x4.cpp lockchecker.cpp memdebug.cpp render.cpp \
                    festring.cpp materialxml.cpp configuration.cpp extensions.cpp selectorxml.cpp \
                    gc_ptr.cpp action.cpp curl.cpp tracker.cpp compositor.cpp core.cpp guitest.cpp \
                    widgets.cpp containers.cpp
nightfall_LDFLAGS = $(LIBINTL)
