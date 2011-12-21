CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(amalgamate)

INCLUDE_DIRECTORIES(./inc /usr/include/ImageMagick)


AUX_SOURCE_DIRECTORY(./src/libamalgamate libamalgamate_src)

ADD_LIBRARY(amalgamatecommon STATIC ${libamalgamate_src})
ADD_EXECUTABLE(amalgamate ./src/amalgamate/main.cpp)
ADD_EXECUTABLE(infinitemosaic ./src/infinitemosaic/main.cpp)
ADD_EXECUTABLE(infinitepanorama ./src/infinitepanorama/infinitepanorama.cpp)
ADD_EXECUTABLE(databasetest ./src/test/databasetest.cpp)
ADD_EXECUTABLE(tiletest ./src/test/tiletest.cpp)
ADD_EXECUTABLE(tilegentest ./src/test/tilegentest.cpp)


TARGET_LINK_LIBRARIES(amalgamatecommon jpeg png boost_filesystem boost_system Magick++)
TARGET_LINK_LIBRARIES(amalgamate boost_program_options amalgamatecommon)
TARGET_LINK_LIBRARIES(infinitemosaic boost_program_options SDL amalgamatecommon)
TARGET_LINK_LIBRARIES(databasetest boost_program_options amalgamatecommon)
TARGET_LINK_LIBRARIES(tiletest boost_program_options amalgamatecommon)
TARGET_LINK_LIBRARIES(tilegentest boost_program_options amalgamatecommon)
TARGET_LINK_LIBRARIES(infinitepanorama boost_program_options Magick++ amalgamatecommon)



#ADD_DEFINITIONS(-g -O2 -fsigned-char -freg-struct-return -Wall -W -Wshadow -Wpointer-arith -Wcast-qual -Winline)
ADD_DEFINITIONS(-g -O3 -fsigned-char -freg-struct-return -Wall)





