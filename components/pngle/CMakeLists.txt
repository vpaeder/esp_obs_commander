idf_component_register(
	SRCS
		"pkg/src/pngle.c"
	INCLUDE_DIRS
		"pkg/src"
)
idf_build_set_property(COMPILE_OPTIONS "-Wno-error=misleading-indentation" APPEND)
