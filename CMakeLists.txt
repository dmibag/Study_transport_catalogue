cmake_minimum_required(VERSION 3.10)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)
if (CMAKE_SYSTEM_NAME MATCHES "^MINGW")
    set(SYSTEM_LIBS -lstdc++)
else()
    set(SYSTEM_LIBS)
endif()

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto transport_router.proto svg.proto map_renderer.proto graph.proto)

set(TC_FILES main.cpp transport_catalogue.h transport_catalogue.cpp transport_catalogue.proto)
set(JSON_FILES json_builder.cpp json.h json_reader.cpp json_reader.h json_builder.h json.cpp)
set(GRAPH_FILES graph.h  router.h transport_router.h transport_router.cpp transport_router.proto graph.proto)
set(OTHER_FILES serialization.cpp serialization.h request_handler.h ranges.h domain.h geo.cpp geo.h svg.cpp svg.h request_handler.cpp map_renderer.h map_renderer.cpp domain.cpp map_renderer.proto svg.proto)

add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${TC_FILES} ${JSON_FILES} ${GRAPH_FILES} ${OTHER_FILES})
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)