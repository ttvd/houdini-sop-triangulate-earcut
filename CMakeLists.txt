cmake_minimum_required(VERSION 3.1)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)
cmake_policy(SET CMP0003 NEW)

project(SOP_TRIANGULATE_EAR_CUT)

cmake_minimum_required(VERSION 3.1)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)
#cmake_policy(SET CMP0003 NEW)
include(CheckCXXCompilerFlag)

# Detect C++11 or C++0x.
#CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
#CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    if(NOT WIN32)
		message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
	endif ()
endif()

# Disable optimization.
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /Od")

# Generate debug info.
set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO} /Zi /DEBUG /OPT:REF /OPT:ICF /INCREMENTAL:NO")

# Function to extract Houdini version from a Version file.
function(extract_houdini_version parm_hfs)
    set(_version_file ${parm_hfs}/toolkit/include/SYS/SYS_Version.h)
    if(EXISTS ${_version_file})
        file(READ ${_version_file} _version_file_contents)
        string(REGEX MATCH "#define SYS_VERSION_FULL \"([0-9]+\.[0-9]+\.[0-9]+)\"" _version_line ${_version_file_contents})
        string(REGEX REPLACE "#define SYS_VERSION_FULL \"([0-9]+\.[0-9]+\.[0-9]+)\"" "\\1" SOP_TRIANGULATE_EAR_CUT_VERSION_FULL ${_version_line})
        set(SOP_TRIANGULATE_EAR_CUT_VERSION_FULL ${SOP_TRIANGULATE_EAR_CUT_VERSION_FULL} PARENT_SCOPE)
    else()
        message(FATAL " ${parm_hfs} does not point to a valid Houdini baseline.")
    endif()
endfunction()

if(APPLE)

    if(DEFINED ENV{HOUDINI_VERSION})
        set(SOP_TRIANGULATE_EAR_CUT_VERSION_FULL $ENV{HOUDINI_VERSION})
        set(SOP_TRIANGULATE_EAR_CUT_HFS /Library/Frameworks/Houdini.Framework/Versions/${SOP_TRIANGULATE_EAR_CUT_VERSION_FULL})
        set(_version_file /Library/Frameworks/Houdini.Framework/Versions/${SOP_TRIANGULATE_EAR_CUT_VERSION_FULL}/Resources/toolkit/include/SYS/SYS_Version.h)
        if(NOT EXISTS ${_version_file})
            message(FATAL " Houdini ${SOP_TRIANGULATE_EAR_CUT_VERSION_FULL} is not installed.")
        endif()
    elseif(DEFINED ENV{HFS})
        set(SOP_TRIANGULATE_EAR_CUT_HFS $ENV{HFS})
        if(EXISTS ${SOP_TRIANGULATE_EAR_CUT_HFS}/Frameworks/Houdini.Framework/Versions/Current/Resources)
            extract_houdini_version(${SOP_TRIANGULATE_EAR_CUT_HFS}/Frameworks/Houdini.Framework/Versions/Current/Resources)
        else()
            extract_houdini_version(${SOP_TRIANGULATE_EAR_CUT_HFS}/Resources)
        endif()
    else()
        message(FATAL " Please define either HOUDINI_VERSION or HFS variable.")
    endif()

    string(REGEX MATCH "([0-9]+\.[0-9]+)" SOP_TRIANGULATE_EAR_CUT_VERSION ${SOP_TRIANGULATE_EAR_CUT_VERSION_FULL})
    set(SOP_TRIANGULATE_EAR_CUT_VERSION ${SOP_TRIANGULATE_EAR_CUT_VERSION})

    # Path to where DSOs are installed for this version.
    set(SOP_TRIANGULATE_EAR_CUT_DSO_PATH $ENV{HOME}/Library/Preferences/houdini/${SOP_TRIANGULATE_EAR_CUT_VERSION}/dso)

    set(SOP_TRIANGULATE_EAR_CUT_INCLUDE_DIRECTORIES
        ${SOP_TRIANGULATE_EAR_CUT_HFS}/Resources/toolkit/include
    )

    set(SOP_TRIANGULATE_EAR_CUT_LINK_DIRECTORIES
        ${SOP_TRIANGULATE_EAR_CUT_HFS}/Libraries
    )

    # Remove lib prefix on OS X.
    set(CMAKE_SHARED_MODULE_PREFIX "")
    set(CMAKE_SHARED_LIBRARY_PREFIX "")

    # Set dylib prefix for modules on OS X.
    set(CMAKE_SHARED_MODULE_SUFFIX ".dylib")

    # OS X specific definitions.
    add_definitions(
        -DVERSION=\"${SOP_TRIANGULATE_EAR_CUT_VERSION_FULL}\"
		-DENABLE_THREADS
		-DUSE_PTHREADS
        -DMBSD_INTEL
        -DMBSD_COCOA
        -DMBSD
        -DUSE_PTHREADS
        -D_GNU_SOURCE
        -DGCC4
        -DGCC3
        "-arch x86_64"
        "-mmacosx-version-min=10.9"
    )

    # Disable rpath.
    set(CMAKE_SKIP_RPATH TRUE)

    # Module creation flags.
    string(REPLACE "-Wl,-headerpad_max_install_names" "" CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS "${CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS}")
    set(CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS "${CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS} -flat_namespace -undefined suppress -arch x86_64 -fobjc-gc")

    set(SOP_TRIANGULATE_EAR_CUT_LIBS
        HoudiniUT
        HoudiniGEO
        HoudiniPRM
        HoudiniOP2
        boost_system
        tbb
    )

elseif (WIN32)
    
    set(SOP_TRIANGULATE_EAR_CUT_HOUDINI_PERFORCE_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../Engine/Plugins/Runtime/Houdini/HoudiniEngine")

    if(EXISTS "${SOP_TRIANGULATE_EAR_CUT_HOUDINI_PERFORCE_LOCATION}")
        set(SOP_TRIANGULATE_EAR_CUT_HFS "${SOP_TRIANGULATE_EAR_CUT_HOUDINI_PERFORCE_LOCATION}")
        extract_houdini_version(${SOP_TRIANGULATE_EAR_CUT_HFS})
    elseif(DEFINED ENV{HOUDINI_VERSION})
        set(SOP_TRIANGULATE_EAR_CUT_VERSION_FULL $ENV{HOUDINI_VERSION})
        set(SOP_TRIANGULATE_EAR_CUT_HFS "C:/Program Files/Side Effects Software/Houdini ${SOP_TRIANGULATE_EAR_CUT_VERSION_FULL}")
        set(_version_file ${SOP_TRIANGULATE_EAR_CUT_HFS}/toolkit/include/SYS/SYS_Version.h)
        if(NOT EXISTS ${_version_file})
            message(FATAL " Houdini ${SOP_TRIANGULATE_EAR_CUT_VERSION_FULL} is not installed.")
        endif()
    elseif(DEFINED ENV{HFS})
        set(SOP_TRIANGULATE_EAR_CUT_HFS $ENV{HFS})
        extract_houdini_version(${SOP_TRIANGULATE_EAR_CUT_HFS})
    else()
        message(FATAL " Please define either HOUDINI_VERSION or HFS variable.")
    endif()

	string(REGEX MATCH "([0-9]+\.[0-9]+)" SOP_TRIANGULATE_EAR_CUT_VERSION ${SOP_TRIANGULATE_EAR_CUT_VERSION_FULL})
    set(SOP_TRIANGULATE_EAR_CUT_VERSION ${SOP_TRIANGULATE_EAR_CUT_VERSION})

    # Path to where DSOs are installed for this version.
    set(SOP_TRIANGULATE_EAR_CUT_DSO_PATH ${SOP_TRIANGULATE_EAR_CUT_HFS}/bin)

    set(SOP_TRIANGULATE_EAR_CUT_INCLUDE_DIRECTORIES
        ${SOP_TRIANGULATE_EAR_CUT_HFS}/toolkit/include
    )

    set(SOP_TRIANGULATE_EAR_CUT_LINK_DIRECTORIES
        ${SOP_TRIANGULATE_EAR_CUT_HFS}/custom/houdini/dsolib
    )

    add_definitions(
        -DVERSION=\"${SOP_TRIANGULATE_EAR_CUT_VERSION_FULL}\"
        -DWIN32
        -DI386
        -nologo
        -TP
        -Zc:forScope
        -DSWAP_BITFIELDS
        -D_WIN32_WINNT=0x0502
        -DNOMINMAX
        -DSTRICT
        -DWIN32_LEAN_AND_MEAN
        -D_USE_MATH_DEFINES
        -D_CRT_SECURE_NO_DEPRECATE
        -D_CRT_NONSTDC_NO_DEPRECATE
        -D_SCL_SECURE_NO_WARNINGS
        -DBOOST_ALL_NO_LIB

        -wd4355
        -w14996
        #-O2
        -DNDEBUG
        -MD
        -EHsc
        -GR
        -bigobj
    )

    set(SOP_TRIANGULATE_EAR_CUT_LIBS
        ${SOP_TRIANGULATE_EAR_CUT_LINK_DIRECTORIES}/*.a
        ${SOP_TRIANGULATE_EAR_CUT_LINK_DIRECTORIES}/*.lib
    )
else()
    message(FATAL_ERROR "Unsupported platform.")
endif()

# Add common definitions.
add_definitions(
    -DMAKING_DSO

    -DAMD64
    -DSIZEOF_VOID_P=8
    -DSESI_LITTLE_ENDIAN
    -DFBX_ENABLED=1
    -DOPENCL_ENABLED=1
    -DOPENVDB_ENABLED=1
    -D_REENTRANT
    -D_FILE_OFFSET_BITS=64
)

# Set include directories.
include_directories(${SOP_TRIANGULATE_EAR_CUT_INCLUDE_DIRECTORIES})
include_directories(earcut/include/mapbox)

# Set link directories.
link_directories(${SOP_TRIANGULATE_EAR_CUT_LINK_DIRECTORIES})

# Set output directory.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})

message(STATUS "HOUDINI_VERSION=${SOP_TRIANGULATE_EAR_CUT_VERSION_FULL}")
message(STATUS "HFS=${SOP_TRIANGULATE_EAR_CUT_HFS}")

add_definitions(
    -DUT_DSO_TAGINFO=\"3262197cbf7248102abe2d9a4e2984bc75108b9aec2dd4b43afa985c1745efd67bde1285db3204751146bfb167c821065ef6ef05ffc2eb54420117f877ea81ac5bbe45a0bdcb5394a374cc4c0ee23b\")


set(SOP_TRIANGULATE_EAR_CUT_INCLUDE
    SOP_TriangulateEarCut.h
)

set(SOP_TRIANGULATE_EAR_CUT_SOURCE
    SOP_TriangulateEarCut.C
)

add_library(SOP_TriangulateEarCut SHARED ${SOP_TRIANGULATE_EAR_CUT_INCLUDE} ${SOP_TRIANGULATE_EAR_CUT_SOURCE})
target_link_libraries(SOP_TriangulateEarCut ${SOP_TRIANGULATE_EAR_CUT_LIBS})
