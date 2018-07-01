cmake_minimum_required(VERSION 2.8)

project(school-compressor)
add_executable(
	${PROJECT_NAME}
	"main.c" "types.h" "result.h"
	"tests.c" "tests.h"
	"linked_list.c" "linked_list.h"
	"quicksort.c" "quicksort.h"
	"huffman.c" "huffman.h"
	"file.c" "file.h"
)
