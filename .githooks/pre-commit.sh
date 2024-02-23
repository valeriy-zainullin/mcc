#!/bin/bash

staged_cpp_files=$(git diff --diff-filter=d --name-only --staged | grep -E '*.cpp$')
staged_cpp_files_num=$(echo "$staged_cpp_files" | wc -l)

if [ staged_cpp_files_num -ne 0 ]; then
  clang-format "$(staged_cpp_files)"
fi
