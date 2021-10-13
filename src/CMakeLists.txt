
find_package(EdupalsBase REQUIRED)
find_package(EdupalsN4D REQUIRED)

add_library(libnss_lliurex SHARED lliurex.cpp)
target_link_libraries(libnss_lliurex Edupals::Base Edupals::N4D)
set_target_properties(libnss_lliurex PROPERTIES SOVERSION 1 VERSION "1.0.0")

install(TARGETS libnss_lliurex LIBRARY DESTINATION "lib")
