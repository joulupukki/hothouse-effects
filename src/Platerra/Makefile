# Project Name
TARGET = platerra

# Uncomment to use LGPL (like ReverbSc, etc.)
#USE_DAISYSP_LGPL=1

#DEBUG=1

# Sources and Hothouse header files
CPP_SOURCES = platerra.cpp ../hothouse.cpp

# Add PlateauNEVersio sources
CPP_SOURCES += ../../third-party/PlateauNEVersio/utilities/Utilities.cpp
CPP_SOURCES += ../../third-party/PlateauNEVersio/dsp/filters/OnePoleFilters.cpp
CPP_SOURCES += ../../third-party/PlateauNEVersio/dsp/delays/InterpDelay.cpp
CPP_SOURCES += ../../third-party/PlateauNEVersio/Dattorro.cpp

C_INCLUDES = -I.. -I../../third-party/PlateauNEVersio

# Library Locations
LIBDAISY_DIR = ../../libDaisy
DAISYSP_DIR = ../../DaisySP

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

# Global helpers
# include ../Makefile
