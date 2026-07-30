if(BUILD_CLIENT)
	add_subdirectory(client)

	if(BUILD_UI)
		add_subdirectory(ui)
	endif()

	set(test_client_SRCS
		${test_client_SRCS}
		${test_ui_SRCS}
		PARENT_SCOPE)
endif()

file(GLOB test_HDRS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

set(test_SRCS
	${test_HDRS}
	${CMAKE_CURRENT_SOURCE_DIR}/test.h
	${CMAKE_CURRENT_SOURCE_DIR}/test.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/test_irr_matrix4.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/test_irr_rotation.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/test_k_d_tree.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/test_nodedef.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/test_serveractiveobjectmgr.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/test_translations.cpp
	PARENT_SCOPE)
