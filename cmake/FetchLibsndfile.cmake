include(cmake/Fetchffw3.cmake)   

get_target_property(FFTW3_INCLUDE_DIR fftw3 SOURCE_DIR)

set(FFTW3_FOUND        TRUE  CACHE BOOL   "" FORCE)
set(FFTW3_LIBRARIES    fftw3 CACHE STRING "" FORCE)
set(FFTW3_INCLUDE_DIRS "${ffw3_SOURCE_DIR}/api" CACHE PATH "" FORCE)

FetchContent_Declare(
    libsndfile
    GIT_REPOSITORY https://github.com/libsndfile/libsndfile
    GIT_TAG        1.2.2
)
set(BUILD_TESTING_TMP        ${BUILD_TESTING})

set(ENABLE_PACKAGE_CONFIG OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING         OFF CACHE BOOL "" FORCE)
set(BUILD_PROGRAMS        OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES        OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(libsndfile)
set(BUILD_TESTING         ${BUILD_TESTING_TMP} CACHE BOOL "" FORCE)
