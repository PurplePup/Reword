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

## compile
CXX = g++
CXXFLAGS = -std=c++17 -c -Wall -ggdb -D_DEBUG -I/usr/include
#CXXFLAGS = -std=c++17 -c -Wall -I/usr/include
# -std=c++11  -m64

## link
# working using std regex
LDFLAGS = -Wall -L/usr/lib -L/usr/lib/x86_64-linux-gnu
# working if using boost regex: 
#LDFLAGS = -Wall -L/usr/lib -L/usr/lib/x86_64-linux-gnu  -lboost_thread -lboost_regex 

# older attempts:
# LDFLAGS = -L/usr/lib/x86_64-linux-gnu -lpthread -lboost_thread -lboost_regex
# LDFLAGS = -L/home/ajm/boost/boost_1_73_0/stage/lib -lboost_thread -lboost_regex


all: $(SOURCES) $(BIN)
#	make pcversion pc=1
#	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(PROG_NAME)
#	$(CXX) rewordlist.cpp words2.cpp ../reword/words.cpp ../reword/random.cpp ../reword/helpers.cpp tinyxml/tinystr.cpp tinyxml/tinyxmlerror.cpp tinyxml/tinyxmlparser.cpp tinyxml/tinyxml.cpp -o rewordlist

pcversion: $(SOURCES) $(BIN)

# NOTE: LDFLAGS must come after OBJECTS
$(BIN): $(OBJECTS) 
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@
	
.cpp.o:
	$(CXX) $(CXXFLAGS) $< -o $@

cleanobjs:
	rm -f $(OBJECTS)

clean:
	rm -f -r $(BIN) *~ *.o *.bak
	rm -f -r ../reword/*.o
