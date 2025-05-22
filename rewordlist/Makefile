## Rewordlist utility setup
###########################

PROG_NAME = rewordlist
SOURCES = rewordlist.cpp \
		words2.cpp \
		tinyxml/tinystr.cpp \
		tinyxml/tinyxml.cpp \
		tinyxml/tinyxmlerror.cpp \
		tinyxml/tinyxmlparser.cpp
EXTSOURCES = ../reword/words.cpp \
		../reword/random.cpp \
		../reword/helpers.cpp 

OBJECTS = $(SOURCES:.cpp=.o)
EXTOBJECTS = $(EXTSOURCES:.cpp=.o)
BIN = $(PROG_NAME)

## debug settings
#################

DBGDIR = bin/debug
DBGBIN = $(DBGDIR)/$(BIN)
DBGOBJS = $(addprefix $(DBGDIR)/, $(OBJECTS))
#DBGCXXFLAGS = -std=c++17 -I/usr/include -O0 -DDEBUG -Wall -Werror -Wextra
DBGCXXFLAGS = -std=c++17 -g -I/usr/include -DDEBUG -D_DEBUG -Wall

## release settings
###################

RELDIR = bin/release
RELBIN = $(RELDIR)/$(BIN)
RELOBJS = $(addprefix $(RELDIR)/, $(OBJECTS))
#RELCXXFLAGS = -std=c++17 -I/usr/include -DNDEBUG -Wall -Werror -Wextra
RELCXXFLAGS = -std=c++17 -I/usr/include -DNDEBUG -Wall

## compile rules
################

CXX = g++

## link rules
#############

# working using std regex
LDFLAGS = -L/usr/lib -L/usr/lib/x86_64-linux-gnu
# working if using boost regex: 
#LDFLAGS = -Wall -L/usr/lib -L/usr/lib/x86_64-linux-gnu  -lboost_thread -lboost_regex 

# older attempts:
# LDFLAGS = -L/usr/lib/x86_64-linux-gnu -lpthread -lboost_thread -lboost_regex
# LDFLAGS = -L/home/ajm/boost/boost_1_73_0/stage/lib -lboost_thread -lboost_regex

## targets
##########

all: prep release

alldebug: prep debug

## Debug rules
##############
debug: $(DBGBIN)

$(DBGBIN): $(DBGOBJS) $(EXTOBJECTS)
	$(CXX) $(CXXFLAGS) $(DBGCXXFLAGS) $(LDFLAGS) -o $(DBGBIN) $^

$(DBGDIR)/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(DBGCXXFLAGS) -o $@ $<
%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(DBGCXXFLAGS) -o $@ $<

## Release rules
################

release: $(RELBIN)

$(RELBIN): $(RELOBJS) $(EXTOBJECTS)
	$(CXX) $(CXXFLAGS) $(RELCXXFLAGS) $(LDFLAGS) -o $(RELBIN) $^

$(RELDIR)/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(RELCXXFLAGS) -o $@ $<

## Other rules
##############

prep:
	@mkdir -p $(DBGDIR)/tinyxml $(RELDIR)/tinyxml

## clean up rules
#################

cleanobjs:
	rm -f $(OBJECTS)

clean:
	rm -f -r $(BIN) *~ *.o *.bak
	rm -f -r ../reword/*.o
