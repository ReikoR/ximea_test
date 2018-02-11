cmake_minimum_required(VERSION 3.8)
project(ximea_test)

set(CMAKE_CXX_STANDARD 14)

#set(CMAKE_CXX_FLAGS "-D_GLIBCXX_USE_CXX11_ABI=0")

include_directories(include)
include_directories(xiApi)

link_directories(xiApi)

set(SOURCE_FILES
        main.cpp
        stdafx.h
        include/DisplayWindow.h
        src/DisplayWindow.cpp
        include/Gui.h
        src/Gui.cpp
        include/XimeaCamera.h
        src/XimeaCamera.cpp
        include/Util.h
        src/Util.cpp
        include/DebugRenderer.h
        src/DebugRenderer.cpp
        include/FpsCounter.h
        src/FpsCounter.cpp
        include/SignalHandler.h
        src/SignalHandler.cpp
        include/Canvas.h
        src/Canvas.cpp
        include/Maths.h
        src/Maths.cpp
        include/BaseCamera.h
        include/Blobber.h
        src/Blobber.cpp
        include/ImageProcessor.h
        src/ImageProcessor.cpp
        include/CameraTranslator.h
        src/CameraTranslator.cpp
        include/LookupTable.h
        src/LookupTable.cpp
        include/Vision.h
        src/Vision.cpp
        include/Object.h
        src/Object.cpp
        include/Maths.h
        src/Maths.cpp
        include/SoccerBot.h
        src/SoccerBot.cpp
        include/MouseListener.h
        include/Config.h
        )

add_executable(ximea_test ${SOURCE_FILES})
target_link_libraries(ximea_test xiapi64.lib)