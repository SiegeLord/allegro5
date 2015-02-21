set(CONTROLLER_DB_SOURCES controller_db.c)

set(CONTROLLER_DB_INCLUDE_FILES
    allegro5/allegro_controller_db.h
    )

set_our_header_properties(${CONTROLLER_DB_INCLUDE_FILES})

add_our_addon_library(allegro_controller_db
    "${CONTROLLER_DB_SOURCES};${CONTROLLER_DB_INCLUDE_FILES}"
    "-DALLEGRO_CONTROLLER_DB_SRC"
    "${ALLEGRO_LINK_WITH}"
    )

set_our_framework_properties(allegro_controller_db AllegroControllerDB-${ALLEGRO_SOVERSION})

install_our_headers(${CONTROLLER_DB_INCLUDE_FILES})

add_addon(controller_db)

#-----------------------------------------------------------------------------#
# vi: set ts=8 sts=4 sw=4 et:
