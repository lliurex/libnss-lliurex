
find_package(EdupalsBase REQUIRED)
find_package(EdupalsN4D REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SYSTEMD REQUIRED libsystemd)


add_library(libnss_lliurex SHARED lliurex.cpp)
target_link_libraries(libnss_lliurex Edupals::Base Edupals::N4D ${SYSTEMD_LIBRARIES})
set_target_properties(libnss_lliurex PROPERTIES SOVERSION 1 VERSION "1.0.0")

install(TARGETS libnss_lliurex LIBRARY DESTINATION "lib")
