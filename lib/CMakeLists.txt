
# FetchContent 를 사용하기 위해 include.
include(FetchContent)

Set(FETCHCONTENT_QUIET FALSE) 
# fmt 라이브러리를 불러온다.
FetchContent_Declare(
  Fmt
  GIT_REPOSITORY "https://github.com/fmtlib/fmt"
  GIT_TAG "7.1.3"
  )
FetchContent_MakeAvailable(Fmt)


# set(BOOST_INCLUDE_LIBRARIES thread filesystem system program_options asio)
set(BOOST_ENABLE_CMAKE ON)

# Boost 라이브러리 다운로드 및 설정
FetchContent_Declare(
  Boost
  URL https://boostorg.jfrog.io/artifactory/main/release/1.81.0/source/boost_1_81_0.tar.gz
  URL_MD5 4bf02e84afb56dfdccd1e6aec9911f4b
  DOWNLOAD_EXTRACT_TIMESTAMP ON
)
set(BOOST_INCLUDE_LIBRARIES thread filesystem system program_options asio)

FetchContent_MakeAvailable(Boost)


# client 디렉토리에 있는 모든 파일을 CLIENT_SRC_FILES 변수에 추가한다.
file(GLOB_RECURSE CLIENT_SRC_FILES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/client/*.cc
)

# server 디렉토리에 있는 모든 파일을 CLIENT_SRC_FILES 변수에 추가한다.
file(GLOB_RECURSE SERVER_SRC_FILES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/server/*.cc
)

add_library(client_lib STATIC ${CLIENT_SRC_FILES})
add_library(server_lib STATIC ${SERVER_SRC_FILES})



# client_lib 의 include 경로 지정
target_include_directories(client_lib PUBLIC ${CMAKE_SOURCE_DIR}/include/client ${CMAKE_CURRENT_BINARY_DIR}/../_deps/boost-src)
# server 의 include 경로 지정
target_include_directories(server_lib PUBLIC ${CMAKE_SOURCE_DIR}/include/server ${CMAKE_CURRENT_BINARY_DIR}/../_deps/boost-src)


# lib 의 컴파일 옵션 지정
target_compile_options(client_lib PRIVATE -Wall -Werror)
target_compile_options(server_lib PRIVATE -Wall -Werror)

# lib 를 C++ 17 로 컴파일
target_compile_options(client_lib PRIVATE -Wall -Werror)
target_compile_features(server_lib PRIVATE cxx_std_11)


# 만일 어떤 라이브러리 A 를 참조한다고 할 때
#     A 를 헤더 파일과 구현 내부에서 모두 사용한다면 : PUBLIC
#     A 를 내부 구현에서만 사용하고 헤더 파일에서는 사용하지 않는다면 : PRIVATE
#     A 를 헤더 파일에서만 사용하고 내부 구현에서는 사용하지 않는다면 : INTERFACE

# find_package(Boost REQUIRED COMPONENTS filesystem system)

# if(Boost_FOUND)
#     message(STATUS "Boost version: ${Boost_VERSION}")
#     message(STATUS "Boost include directories: ${Boost_INCLUDE_DIRS}")
#     message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
# else()
#     message(FATAL_ERROR "Boost not found.")
# endif()




# client_lib 에 링크
target_link_libraries(client_lib PUBLIC fmt)

# server_lib 에 링크
target_link_libraries(server_lib PUBLIC )

