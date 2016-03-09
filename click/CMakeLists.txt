
function(configure_scope_files)
  foreach(_file ${ARGV})
    configure_file(
      "${_file}.in"
      "${CMAKE_CURRENT_BINARY_DIR}/${_file}"
      @ONLY
    )
  endforeach()
endfunction()

configure_scope_files(
  manifest.json
  weibo.provider
  weibo.service
  weibo.application
)

add_custom_target(hidden_files1
  ALL
  SOURCES
    apparmor.json
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/manifest.json"
    "apparmor.json"
    "${CMAKE_CURRENT_BINARY_DIR}/weibo.provider"
    "${CMAKE_CURRENT_BINARY_DIR}/weibo.service"
    "${CMAKE_CURRENT_BINARY_DIR}/weibo.application"
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)

install(
  DIRECTORY
    "qml-plugin"
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)
