cmake_minimum_required(VERSION 4.2)
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
file(GLOB BLANK_BIN pl.art.lach.slawek.V2BlankBrowser.svg)
file(GLOB META_INFO pl.art.lach.slawek.V2BlankBrowser.appdata.xml )
file(GLOB DESKTOP_FILE pl.art.lach.slawek.V2BlankBrowser.desktop ) 


add_executable( V2BlankBrowser ${BLANK_SRC} )
target_link_libraries( V2BlankBrowser PUBLIC PkgConfig::GTK  )
INSTALL( TARGETS  V2BlankBrowser RUNTIME DESTINATION bin)
INSTALL( FILES ${DESKTOP_FILE} DESTINATION /app/share/applications )
INSTALL( FILES ${BLANK_BIN} DESTINATION /app/share/icons/hicolor/scalable/apps/ )
INSTALL( FILES ${META_INFO} DESTINATION /app/share/metainfo )
