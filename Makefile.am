bin_PROGRAMS = unfolder unfolder-client addict

unfolder_SOURCES = \
	unfolder.cpp \
	libddag/ddag.cpp \
	lib/connection.cpp \
	lib/client.cpp \
	lib/dag.cpp \
	lib/dag_state.cpp \
	lib/io.cpp \
	lib/machine.cpp \
	lib/machine_list.cpp \
	lib/server.cpp \
	lib/semaphore.cpp \
	libaddict/addict_dag.cpp \
	libaddict/addict_parser.cpp \
	libaddict/reader.cpp \
	libaddict/tools.cpp

unfolder_client_SOURCES = \
	client.cpp \
	libddag/ddag.cpp \
	lib/connection.cpp \
	lib/client.cpp \
	lib/dag.cpp \
	lib/dag_state.cpp \
	lib/io.cpp \
	lib/machine.cpp \
	lib/machine_list.cpp \
	lib/server.cpp \
	lib/semaphore.cpp

addict_SOURCES = \
	libaddict/addict.cpp \
	libaddict/addict_dag.cpp \
	libaddict/addict_parser.cpp \
	libaddict/reader.cpp \
	libaddict/tools.cpp

AM_CXXFLAGS = -Wall -Werror -std=c++11
