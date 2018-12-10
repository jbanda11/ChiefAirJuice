ACLOCAL_AMFLAGS= -I m4

noinst_HEADERS = CollisionContext.h BaseApplication.h GameApplication.h GameManager.h GameObject.h OgreMotionState.h Simulator.h CollisionCallback.h ScriptHandler.h audio.h NetManager.h PlayerMotionState.h

bin_PROGRAMS = ChiefAirJuice
ChiefAirJuice_CPPFLAGS = -I$(top_srcdir)
ChiefAirJuice_SOURCES = BaseApplication.cpp GameApplication.cpp GameManager.cpp GameObject.cpp NetManager.cpp Simulator.cpp audio.c
ChiefAirJuice_CXXFLAGS = $(OGRE_CFLAGS) $(SDL_CFLAGS) $(OIS_CFLAGS) $(bullet_CFLAGS) -I/lusr/opt/cegui-0.8.7/include/cegui-0 -I/usr/include/SDL -I/usr/include/SDL2
ChiefAirJuice_LDADD = $(OGRE_LIBS) $(SDL_LIBS) $(OIS_LIBS) $(bullet_LIBS) -L/lusr/opt/cegui-0.8.7/lib -lCEGUIBase-0 -lCEGUIOgreRenderer-0
ChiefAirJuice_LDFLAGS = -lOgreOverlay -lboost_system -lSDL_net -lSDL2main -lSDL2


EXTRA_DIST = buildit makeit
AUTOMAKE_OPTIONS = foreign