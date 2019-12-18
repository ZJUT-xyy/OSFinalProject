cmake_minimum_required(VERSION 3.15)
project(OSFinal1)

set(CMAKE_CXX_STANDARD 14)

add_executable(OSFinal1 main.cpp UnixFileSys.h Entity/SuperBlock.h Entity/DINode.h Entity/INode.h Entity/Direct.h Entity/Dir.h Entity/Owner.h Entity/Owners.h Entity/Group.h Entity/Groups.h UnixFIleSys.cpp)