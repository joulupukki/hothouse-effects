# Project Name
TARGET = flick

# Uncomment to use LGPL (like ReverbSc, etc.)
# USE_DAISYSP_LGPL=1

# Sources and Hothouse header files
CPP_SOURCES = flick.cpp ../hothouse.cpp

# Add PlateauNEVersio sources
CPP_SOURCES += ../../third-party/PlateauNEVersio/utilities/Utilities.cpp
CPP_SOURCES += ../../third-party/PlateauNEVersio/dsp/filters/OnePoleFilters.cpp
CPP_SOURCES += ../../third-party/PlateauNEVersio/dsp/delays/InterpDelay.cpp
CPP_SOURCES += ../../third-party/PlateauNEVersio/Dattorro.cpp
# CPP_SOURCES += ../../third-party/PlateauNEVersio/src/bogaudio.cpp
# CPP_SOURCES += ../../third-party/PlateauNEVersio/src/Lmtr.cpp
# CPP_SOURCES += ../../third-party/PlateauNEVersio/src/utils.cpp
# CPP_SOURCES += ../../third-party/PlateauNEVersio/src/dsp/analyzer.cpp
# CPP_SOURCES += ../../third-party/PlateauNEVersio/src/dsp/math.cpp
# CPP_SOURCES += ../../third-party/PlateauNEVersio/src/dsp/signal.cpp
# CPP_SOURCES += ../../third-party/PlateauNEVersio/src/dsp/table.cpp
# CPP_SOURCES += ../../third-party/PlateauNEVersio/src/dsp/filters/filter.cpp
# CPP_SOURCES += ../../third-party/PlateauNEVersio/src/dsp/filters/utility.cpp

C_INCLUDES = -I.. -I../../third-party/PlateauNEVersio

# Library Locations
LIBDAISY_DIR = ../../libDaisy
DAISYSP_DIR = ../../DaisySP

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

# Global helpers
# include ../Makefile
