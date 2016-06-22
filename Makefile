CC = $(CROSS)g++ $(foreach sysroot,$(SYSROOT),--sysroot=$(sysroot))
AR = $(CROSS)ar
CFLAGS = -W -pthread -g -std=c++11 -Iinc
LDFLAGS = -pthread 

WEBRTCROOT?=/media/fitra/656d385f-4f1c-4b14-a3ee-4f0b172196f2/webrtc
WEBRTCBUILD?=Release
WEBRTCLIBPATH=$(WEBRTCROOT)/src/out/$(WEBRTCBUILD)

CFLAGS += -DWEBRTC_POSIX -fno-rtti -D_GLIBCXX_USE_CXX11_ABI=0
CFLAGS += -I $(WEBRTCROOT)/src -I $(WEBRTCROOT)/src/chromium/src/third_party/jsoncpp/source/include

LDFLAGS += -lX11 -ldl -lrt

TARGET = camrtc-server

all: $(TARGET)

WEBRTC_LIB = $(shell find $(WEBRTCLIBPATH) -name '*.a')

libjingle_peerconnection.a: $(WEBRTC_LIB)
	$(AR) -rcT $@ $^

src/%.o: src/%.cpp
	$(CC) -o $@ -c $^ $(CFLAGS) 

FILES = $(wildcard src/*.cpp)

$(TARGET): $(subst .cpp,.o,$(FILES)) libjingle_peerconnection.a
	$(CC) -o $@ $^ $(LDFLAGS) 

clean:
	rm -f src/*.o libjingle_peerconnection.a $(TARGET)
