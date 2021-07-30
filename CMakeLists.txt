cmake_minimum_required(VERSION 3.16.3)

project(tcp-echo VERSION 1.0)

add_compile_options(-Wall)

add_executable("server" server.c)
add_executable("client" client.c)
