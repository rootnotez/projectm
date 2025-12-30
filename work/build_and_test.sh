#!/bin/bash
set -e

# Script to compile and run test_issue_940
# Can be run from the work directory

echo "Compiling test_issue_940..."
cd ../vendor/hlslparser

g++ -std=c++14 -I./src -o ../../work/test_issue_940 \
  ../../work/test_issue_940.cpp \
  src/Engine.cpp \
  src/HLSLTokenizer.cpp \
  src/HLSLParser.cpp \
  src/HLSLTree.cpp \
  src/CodeWriter.cpp \
  src/GLSLGenerator.cpp

echo "✓ Compilation successful!"
echo ""
echo "Running tests..."
cd ../../work
./test_issue_940
