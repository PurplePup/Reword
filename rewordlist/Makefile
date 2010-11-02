PROG_NAME = rewordlist
SOURCES = rewordlist.cpp \
		words2.cpp \
		tinyxml/tinystr.cpp \
		tinyxml/tinyxml.cpp \
		tinyxml/tinyxmlerror.cpp \
		tinyxml/tinyxmlparser.cpp \
		../reword/words.cpp \
		../reword/random.cpp \
		../reword/helpers.cpp

OBJECTS = $(SOURCES:.cpp=.o)
BIN = $(PROG_NAME)

CXX = g++
CXXFLAGS = -Wall -ansi -g
LDFLAGS = -c -Wall -ggdb -D_DEBUG

all:
#	make pcversion pc=1
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(PROG_NAME)
#	$(CXX) rewordlist.cpp words2.cpp ../reword/words.cpp ../reword/random.cpp ../reword/helpers.cpp tinyxml/tinystr.cpp tinyxml/tinyxmlerror.cpp tinyxml/tinyxmlparser.cpp tinyxml/tinyxml.cpp -o rewordlist

pcversion: $(SOURCES) $(BIN)

cleanobjs:
	rm -f $(OBJECTS)

clean:
	rm -f $(BIN) *~ *.o *.bak

$(BIN): $(OBJECTS) 
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@
	
.cpp.o:
	$(CXX) $(CXXFLAGS) $< -o $@


