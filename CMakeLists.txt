cmake_minimum_required(VERSION 2.8)

project(school-compressor)
add_executable(
	${PROJECT_NAME}
	"main.c" "types.h"
	"huffman.c" "huffman.h"
	"linked-list-quicksort.c" "linked-list-quicksort.h"
	"linked-list.c" "linked-list.h"
	"tests.c" "tests.h"
)
