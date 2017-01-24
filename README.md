Strinalyze - Analyze a string or a sequence of generated strings
================================================================

This tool prints common string index data structures like the suffix array, the LCP array, the BWT, etc.
One can pass a string as a parameter to the program, or select a string sequence generator.
Currently, the generators for Fibonacci words and similar sequences are available.

# Dependencies

- Command line tools
  - cmake
  - make
  - a C++11 compiler like gcc or clang 
  - git and svn to clone and build the external dependecies

The following dependencies will be downloaded by cmake and used to compile this project:
- glog
- gtest

The program uses Yuta Mori's SAIS algorithm (https://sites.google.com/site/yuta256/sais) for computing the suffix array.
