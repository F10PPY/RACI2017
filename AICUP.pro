TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CXXFLAGS += -std=c++14 -static \
     -fno-optimize-sibling-calls -fno-strict-aliasing -D_LINUX \
    -lm -s -O0 -Wall -Wtype-limits -Wno-unknown-pragmas
LIBS += -L/usr/include -lncurses
#cmake_minimum_required(VERSION 3.5)
#project(MyStrategy)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -static \
#    -fno-optimize-sibling-calls -fno-strict-aliasing -D_LINUX \
#    -lm -s -O2 -Wall -Wtype-limits -Wno-unknown-pragmas")

#file(GLOB strategy_SRC "*.cpp" "model/*.cpp" "csimplesocket/*.cpp")

#add_executable(MyStrategy ${strategy_SRC})

SOURCES += Strategy.cpp \
    Runner.cpp \
    RemoteProcessClient.cpp \
    csimplesocket/ActiveSocket.cpp \
    csimplesocket/HTTPActiveSocket.cpp \
    csimplesocket/PassiveSocket.cpp \
    csimplesocket/SimpleSocket.cpp \
    model/CircularUnit.cpp \
    model/Facility.cpp \
    model/Game.cpp \
    model/Move.cpp \
    model/Player.cpp \
    model/PlayerContext.cpp \
    model/Unit.cpp \
    model/Vehicle.cpp \
    model/VehicleUpdate.cpp \
    model/World.cpp \
    MyStrategy.cpp \


HEADERS += \
       Strategy.h \
    Runner.h \
    RemoteProcessClient.h \
    MyStrategy.h \
    csimplesocket/ActiveSocket.h \
    csimplesocket/Host.h \
    csimplesocket/HTTPActiveSocket.h \
    csimplesocket/PassiveSocket.h \
    csimplesocket/SimpleSocket.h \
    csimplesocket/StatTimer.h \
    model/ActionType.h \
    model/CircularUnit.h \
    model/Facility.h \
    model/FacilityType.h \
    model/Game.h \
    model/Move.h \
    model/Player.h \
    model/PlayerContext.h \
    model/TerrainType.h \
    model/Unit.h \
    model/Vehicle.h \
    model/VehicleType.h \
    model/VehicleUpdate.h \
    model/WeatherType.h \
    model/World.h \
    MyStrategy.h \
    RemoteProcessClient.h \
    Runner.h \

SUBDIRS += \
    test.pro \
    AICUP.pro

DISTFILES += \
    MyStrategy \
    compile-g++14.sh \
    CMakeLists.txt.user \
    AICUP.pro.user \
    cpp-cgdk.vcxproj.filters \
    cpp-cgdk.vcxproj \
    cpp-cgdk.sln \
    compile-vscpp.bat \
    compile-g++14.bat \
    CMakeLists.txt \
    compilation.log
