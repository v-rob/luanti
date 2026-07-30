file(GLOB test_ui_HDRS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

set(test_ui_SRCS
	${test_ui_HDRS}
	PARENT_SCOPE)
