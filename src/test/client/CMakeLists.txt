file(GLOB test_client_HDRS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

set(test_client_SRCS
	${test_client_HDRS}
	${CMAKE_CURRENT_SOURCE_DIR}/test_irr_gltf_mesh_loader.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/test_irr_x_mesh_loader.cpp
	PARENT_SCOPE)
