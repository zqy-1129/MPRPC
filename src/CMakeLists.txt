aux_source_directory(. SRC_LIST)

add_library(mprpc ${SRC_LIST})

# 链接库（muduo pthread zookeeper）
target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt)