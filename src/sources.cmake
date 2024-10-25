# This file was automatically generated by scripts/update_sources.py
target_sources(${PROJECT_NAME} PRIVATE
    libtcod-fov/bresenham.h
    libtcod-fov/bresenham.hpp
    libtcod-fov/bresenham_c.c
    libtcod-fov/config.h
    libtcod-fov/error.c
    libtcod-fov/error.h
    libtcod-fov/error.hpp
    libtcod-fov/fov.h
    libtcod-fov/fov.hpp
    libtcod-fov/fov_c.c
    libtcod-fov/fov_circular_raycasting.c
    libtcod-fov/fov_diamond_raycasting.c
    libtcod-fov/fov_permissive2.c
    libtcod-fov/fov_recursive_shadowcasting.c
    libtcod-fov/fov_restrictive.c
    libtcod-fov/fov_symmetric_shadowcast.c
    libtcod-fov/fov_types.h
    libtcod-fov/libtcod_int.h
    libtcod-fov/logging.c
    libtcod-fov/logging.h
    libtcod-fov/map_inline.h
    libtcod-fov/map_types.h
    libtcod-fov/utility.h
    libtcod-fov/version.h
)
install(FILES
    libtcod-fov.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/.
    COMPONENT IncludeFiles
)
install(FILES
    libtcod-fov/bresenham.h
    libtcod-fov/bresenham.hpp
    libtcod-fov/config.h
    libtcod-fov/error.h
    libtcod-fov/error.hpp
    libtcod-fov/fov.h
    libtcod-fov/fov.hpp
    libtcod-fov/fov_types.h
    libtcod-fov/libtcod_int.h
    libtcod-fov/logging.h
    libtcod-fov/map_inline.h
    libtcod-fov/map_types.h
    libtcod-fov/utility.h
    libtcod-fov/version.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libtcod-fov
    COMPONENT IncludeFiles
)
source_group(libtcod-fov FILES
    libtcod-fov/bresenham.h
    libtcod-fov/bresenham.hpp
    libtcod-fov/bresenham_c.c
    libtcod-fov/config.h
    libtcod-fov/error.c
    libtcod-fov/error.h
    libtcod-fov/error.hpp
    libtcod-fov/fov.h
    libtcod-fov/fov.hpp
    libtcod-fov/fov_c.c
    libtcod-fov/fov_circular_raycasting.c
    libtcod-fov/fov_diamond_raycasting.c
    libtcod-fov/fov_permissive2.c
    libtcod-fov/fov_recursive_shadowcasting.c
    libtcod-fov/fov_restrictive.c
    libtcod-fov/fov_symmetric_shadowcast.c
    libtcod-fov/fov_types.h
    libtcod-fov/libtcod_int.h
    libtcod-fov/logging.c
    libtcod-fov/logging.h
    libtcod-fov/map_inline.h
    libtcod-fov/map_types.h
    libtcod-fov/utility.h
    libtcod-fov/version.h
)
