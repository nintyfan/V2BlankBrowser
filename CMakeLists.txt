cmake_minimum_required(VERSION 2.8.9)
project(BlankBrowser)
get_property(LIB64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS)



if ("${LIB64}" STREQUAL "TRUE")

    set(LIBSUFFIX 64)

else()

  set(LIBSUFFIX "")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED IMPORTED_TARGET gtk+-3.0 webkit2gtk-4.0  )
file(GLOB BLANK_SRC main.c)


add_executable( BlankBrowser ${BLANK_SRC} )
target_link_libraries( BlankBrowser PUBLIC PkgConfig::GTK  )
INSTALL( TARGETS  BlankBrowser RUNTIME DESTINATION bin)

