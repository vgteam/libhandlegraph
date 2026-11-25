find_package(OpenMP QUIET COMPONENTS CXX)
if(NOT OpenMP_CXX_FOUND AND CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang"
   AND NOT CMAKE_DISABLE_FIND_PACKAGE_OpenMP)
    # AppleClang needs the separately installed libomp from Homebrew or MacPorts.
    find_path(HANDLEGRAPH_OMP_INCLUDE omp.h
        HINTS "$ENV{HOMEBREW_PREFIX}/opt/libomp/include" /opt/homebrew/opt/libomp/include /usr/local/opt/libomp/include /opt/local/include/libomp)
    find_library(HANDLEGRAPH_OMP_LIBRARY omp
        HINTS "$ENV{HOMEBREW_PREFIX}/opt/libomp/lib" /opt/homebrew/opt/libomp/lib /usr/local/opt/libomp/lib /opt/local/lib/libomp)
    if(HANDLEGRAPH_OMP_INCLUDE AND HANDLEGRAPH_OMP_LIBRARY)
        set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp")
        set(OpenMP_CXX_INCLUDE_DIR "${HANDLEGRAPH_OMP_INCLUDE}")
        set(OpenMP_CXX_LIB_NAMES omp)
        set(OpenMP_omp_LIBRARY "${HANDLEGRAPH_OMP_LIBRARY}")
        find_package(OpenMP QUIET COMPONENTS CXX)
    endif()
endif()
