CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(amalgamate)

INCLUDE_DIRECTORIES(./inc ./TbdLib/include /usr/include/ImageMagick)

AUX_SOURCE_DIRECTORY(./src/libgist libgist_src)
AUX_SOURCE_DIRECTORY(./src/libamalgamate libamalgamate_src)
AUX_SOURCE_DIRECTORY(./src/libgraphcut libgraphcut_src)
AUX_SOURCE_DIRECTORY(./src/libpoisson libpoisson_src)

ADD_LIBRARY(gist STATIC ${libgist_src})
ADD_LIBRARY(graphcut STATIC ${libgraphcut_src})
ADD_LIBRARY(poisson STATIC ${libpoisson_src})
ADD_LIBRARY(amalgamatecommon SHARED ${libamalgamate_src})

ADD_EXECUTABLE(amalgamate ./src/amalgamate/main.cpp)
ADD_EXECUTABLE(infinitepanorama ./src/infinitepanorama/infinitepanorama.cpp ./src/infinitepanorama/Panorama.cpp)


TARGET_LINK_LIBRARIES(graphcut)
TARGET_LINK_LIBRARIES(gist fftw3f)
TARGET_LINK_LIBRARIES(amalgamatecommon gist jpeg png boost_filesystem boost_system Magick++ gomp)
TARGET_LINK_LIBRARIES(amalgamate boost_program_options amalgamatecommon)
TARGET_LINK_LIBRARIES(infinitepanorama boost_program_options Magick++ amalgamatecommon glut GLU GL graphcut cv cxcore poisson highgui csparse cxsparse umfpack)

# Tests
ADD_EXECUTABLE(databasetest ./src/test/databasetest.cpp)
ADD_EXECUTABLE(tiletest ./src/test/tiletest.cpp)
ADD_EXECUTABLE(tilegentest ./src/test/tilegentest.cpp)
ADD_EXECUTABLE(gisttest ./src/test/GISTTest.cpp)
ADD_EXECUTABLE(filtertest ./src/test/DescriptorFilterTest.cpp)
ADD_EXECUTABLE(graphcuttest ./src/test/GraphCutTest.cpp)

TARGET_LINK_LIBRARIES(databasetest boost_program_options amalgamatecommon)
TARGET_LINK_LIBRARIES(tiletest boost_program_options amalgamatecommon)
TARGET_LINK_LIBRARIES(tilegentest boost_program_options amalgamatecommon)
TARGET_LINK_LIBRARIES(gisttest boost_program_options amalgamatecommon)
TARGET_LINK_LIBRARIES(filtertest boost_program_options amalgamatecommon)
TARGET_LINK_LIBRARIES(graphcuttest boost_program_options Magick++ graphcut cv cxcore highgui poisson csparse umfpack cxsparse)

#ADD_DEFINITIONS(-g -O2 -fsigned-char -freg-struct-return -Wall -W -Wshadow -Wpointer-arith -Wcast-qual -Winline)
ADD_DEFINITIONS(-g -O3 -fsigned-char -fopenmp -freg-struct-return -Wall -fPIC)


