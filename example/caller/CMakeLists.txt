# set(SRC_LIST CallUserService.cc ../User.pb.cc)
set(SRC_LIST CallFriendService.cc ../Friend.pb.cc)

add_executable(consumer ${SRC_LIST})

target_link_libraries(consumer mprpc protobuf)