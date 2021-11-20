cmake_minimum_required(VERSION 2.8.9)
project(V2BlankBrowser)
get_property(LIB64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS)



if ("${LIB64}" STREQUAL "TRUE")

    set(LIBSUFFIX 64)

else()

  set(LIBSUFFIX "")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED IMPORTED_TARGET gtk+-3.0 webkit2gtk-4.0  )
file(GLOB BLANK_SRC main.c download.c )
file(GLOB BLANK_BIN logo.png)
file(GLOB META_INFO pl.art.lach.slawek.apps.V2BlankBrowser.appdata.xml )


add_executable( V2BlankBrowser ${BLANK_SRC} )
target_link_libraries( V2BlankBrowser PUBLIC PkgConfig::GTK  )
INSTALL( TARGETS  V2BlankBrowser RUNTIME DESTINATION bin)
INSTALL( FILES ${BLANK_BIN} DESTINATION /app/share/icons/hicolor/48x48/apps/ )
INSTALL( FILES ${META_INFO} DESTINATION /app/share/metainfo )
