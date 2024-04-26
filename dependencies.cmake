include(FetchContent)

FetchContent_Declare(
    cmakehelpers
    GIT_REPOSITORY      https://github.com/halex2005/CMakeHelpers.git
    GIT_TAG             0814803829022ce3f326b14348971ff556a1dee3 #
)

FetchContent_MakeAvailable(cmakehelpers)
list(APPEND CMAKE_MODULE_PATH "${cmakehelpers_SOURCE_DIR}")

#----------------------------------------------------------------------------------------

FetchContent_Declare(
    tclap
    GIT_REPOSITORY      https://github.com/mirror/tclap.git
    GIT_TAG             799a8b1f99818e39fee19d0601030770af1221e1 #v1.4.0-rc1
    PATCH_COMMAND       git apply "${CMAKE_SOURCE_DIR}/patches/tclap.patch"
    UPDATE_DISCONNECTED 1
)
FetchContent_MakeAvailable(tclap)

#----------------------------------------------------------------------------------------

FetchContent_Declare(
  Boost
  URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
  URL_MD5 893b5203b862eb9bbd08553e24ff146a
  DOWNLOAD_EXTRACT_TIMESTAMP ON
  EXCLUDE_FROM_ALL
)
set(BOOST_INCLUDE_LIBRARIES process)
FetchContent_MakeAvailable(Boost)

#----------------------------------------------------------------------------------------

FetchContent_Declare(
    3dstool
    URL         https://github.com/dnasdw/3dstool/releases/download/v1.0.9/3dstool.zip
)

FetchContent_Declare(
    ctrtool
    URL         https://github.com/3DSGuy/Project_CTR/releases/download/ctrtool-v0.5/ctrtool-win_x86_64-v0.5.zip
)

FetchContent_Declare(
    makerom
    URL         https://github.com/3DSGuy/Project_CTR/releases/download/makerom-v0.15/makerom-win_x86_64-v0.15.zip
)

FetchContent_MakeAvailable(3dstool ctrtool makerom)

#----------------------------------------------------------------------------------------

install(FILES ${3dstool_SOURCE_DIR}/3dstool.exe DESTINATION tools)
install(FILES ${ctrtool_SOURCE_DIR}/ctrtool.exe DESTINATION tools)
install(FILES ${makerom_SOURCE_DIR}/makerom.exe DESTINATION tools)