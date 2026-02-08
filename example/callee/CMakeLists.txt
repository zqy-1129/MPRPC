# set(SRC_LIST UserService.cc ../User.pb.cc)
set(SRC_LIST FriendService.cc ../Friend.pb.cc)

add_executable(provider ${SRC_LIST})

target_link_libraries(provider mprpc protobuf)