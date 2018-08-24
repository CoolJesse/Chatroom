# using variables
COMPILER := /usr/bin/g++
#FLAGS :=
FLAGS := -Wall
# FLAGS := -D_GLIBCXX_USE_CXX11_ABI=1
OBJECTS := helper.o user_info.o

PHONY: all

all: server_threaded client_threaded helper.o user_info.o active_user_info.o

server_threaded: server_threaded.cpp $(OBJECTS) active_user_info.o
	$(COMPILER) $(FLAGS) -o server_threaded server_threaded.cpp user_info.o active_user_info.o helper.o -lpthread

client_threaded: client_threaded.cpp $(OBJECTS)
	$(COMPILER) $(FLAGS) -o client_threaded client_threaded.cpp user_info.o helper.o -lpthread

user_info.o: user_info.h user_info.cpp
	$(COMPILER) $(FLAGS) -c user_info.cpp

active_user_info.o: active_user_info.h active_user_info.cpp
	$(COMPILER) $(FLAGS) -c active_user_info.cpp

helper.o: helper.h helper.cpp user_info.o
	$(COMPILER) $(FLAGS) -c helper.cpp user_info.cpp

