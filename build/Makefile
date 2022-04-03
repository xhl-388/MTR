SYSCONF_LINK = g++
CPPFLAGS     = -std=c++17 -Wall -O2
CFLAGS		 =
LDFLAGS      =
LIBS         = -lm

MODE	= RELEASE
DESTDIR = ./
TARGET  = $(DESTDIR)main
SOURCEDIR = ../
SOURCE	= $(wildcard $(SOURCEDIR)*.cpp)
OBJECTS = $(addprefix $(DESTDIR),$(notdir $(patsubst %.cpp,%.o,$(SOURCE))))

ifeq ($(MODE),DEBUG)
CPPFLAGS	+=	-O0	-g2
endif

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(SYSCONF_LINK) $(CPPFLAGS)  $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

$(OBJECTS): $(DESTDIR)%.o: $(SOURCEDIR)%.cpp
	$(SYSCONF_LINK) $(CPPFLAGS) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	-rm -f $(OBJECTS)
	-rm -f $(TARGET)

