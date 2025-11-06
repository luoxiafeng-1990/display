AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = display_test

display_test_SOURCES = test.cpp \
                       source/LinuxFramebufferDevice.cpp \
                       source/VideoFile.cpp \
                       source/PerformanceMonitor.cpp

AM_CPPFLAGS = -I$(top_srcdir)/include

if DEBUG
AM_CXXFLAGS = -g -O0 -Wall -std=c++11
else
AM_CXXFLAGS = -O2 -std=c++11
endif

LDADD = -lpthread

