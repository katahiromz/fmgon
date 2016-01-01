# Makefile.g++

DOTOBJ = .o
DOTEXE = .exe   # Windows
#DOTEXE =       # Linux

EXEFILE = fmgon_test$(DOTEXE)

COMMON_HEADERS = \
	stdafx.h \
	misc.h \

OBJS = \
	fmgen$(DOTOBJ) \
	fmtimer$(DOTOBJ) \
	opm$(DOTOBJ) \
	opna$(DOTOBJ) \
	posix_pevent$(DOTOBJ) \
	psg$(DOTOBJ) \
	soundplayer$(DOTOBJ) \
	YM2203$(DOTOBJ) \
	YM2203_Timbre$(DOTOBJ) \

CXX = g++ -std=c++11
CXXFLAGS = -mconsole -DNDEBUG -DSOUND_TEST

LIBS=-lalut -lopenal32
#LIBS=-lalut -lopenal32.dll -static

all: $(EXEFILE)

$(EXEFILE): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(EXEFILE) $(OBJS) $(LIBS)

fmgen$(DOTOBJ): $(COMMON_HEADERS) fmgen.cpp fmgen.h fmgeninl.h
	$(CXX) -c $(CXXFLAGS) -o fmgen$(DOTOBJ) fmgen.cpp
fmtimer$(DOTOBJ): $(COMMON_HEADERS) fmtimer.cpp fmtimer.h
	$(CXX) -c $(CXXFLAGS) -o fmtimer$(DOTOBJ) fmtimer.cpp
opm$(DOTOBJ): $(COMMON_HEADERS) opm.cpp opm.h fmgen.h fmtimer.h psg.h fmgeninl.h
	$(CXX) -c $(CXXFLAGS) -o opm$(DOTOBJ) opm.cpp
opna$(DOTOBJ): $(COMMON_HEADERS) opna.cpp opna.h fmgen.h fmtimer.h psg.h fmgeninl.h
	$(CXX) -c $(CXXFLAGS) -o opna$(DOTOBJ) opna.cpp
posix_pevent$(DOTOBJ): $(COMMON_HEADERS) posix_pevent.cpp
	$(CXX) -c $(CXXFLAGS) -o posix_pevent$(DOTOBJ) posix_pevent.cpp
psg$(DOTOBJ): $(COMMON_HEADERS) psg.cpp psg.h
	$(CXX) -c $(CXXFLAGS) -o psg$(DOTOBJ) psg.cpp
soundplayer$(DOTOBJ): $(COMMON_HEADERS) soundplayer.cpp soundplayer.h YM2203.h
	$(CXX) -c $(CXXFLAGS) -o soundplayer$(DOTOBJ) soundplayer.cpp
YM2203$(DOTOBJ): $(COMMON_HEADERS) YM2203.cpp YM2203.h opna.h YM2203_Timbre.h
	$(CXX) -c $(CXXFLAGS) -o YM2203$(DOTOBJ) YM2203.cpp
YM2203_Timbre$(DOTOBJ): $(COMMON_HEADERS) YM2203_Timbre.cpp YM2203_Timbre.h
	$(CXX) -c $(CXXFLAGS) -o YM2203_Timbre$(DOTOBJ) YM2203_Timbre.cpp

clean:
	rm -f *.o
