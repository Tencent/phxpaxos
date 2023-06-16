aux_source_directory(logger_google PLUGIN_SOURCES)
aux_source_directory(monitor PLUGIN_SOURCES)
message(STATUS "[LOG/plugin] PLUGIN_SOURCES: ${PLUGIN_SOURCES}")
add_library(plugin ${PLUGIN_SOURCES})
