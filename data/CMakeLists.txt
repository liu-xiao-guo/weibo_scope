intltool_merge_translations(
  "${CMAKE_CURRENT_SOURCE_DIR}/${SCOPE_NAME}.ini.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${SCOPE_INSTALL_NAME}.ini"
  ALL
  UTF8
)

message("SCOPE_INSTALL_NAME: " ${SCOPE_INSTALL_NAME})

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/${SCOPE_INSTALL_NAME}.ini"
  DESTINATION ${SCOPE_INSTALL_DIR}
)

# Install the scope images
install(
  FILES
    "icon.png"
    "logo.png"
    "screenshot.png"
  DESTINATION
    "${SCOPE_INSTALL_DIR}"
)

