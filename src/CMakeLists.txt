
find_package(EdupalsBase REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SYSTEMD REQUIRED libsystemd)
pkg_check_modules(CURL REQUIRED libcurl)


add_library(nss_lliurex SHARED lliurex.cpp http.cpp)
target_link_libraries(nss_lliurex Edupals::Base ${SYSTEMD_LIBRARIES} ${CURL_LIBRARIES})
set_target_properties(nss_lliurex PROPERTIES SOVERSION 2 VERSION "2.0.0")

install(TARGETS nss_lliurex LIBRARY DESTINATION "lib")
