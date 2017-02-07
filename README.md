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


# Example

> ./strinalyze -ex 'babarabar'
~~~~
T: babarabar
|T|: 9
   i  1  2  3  4  5  6  7  8  9 10
   T  b  a  b  a  r  a  b  a  r  $
  SA 10  6  2  8  4  1  7  3  9  5
 LCP  0  0  4  1  2  0  2  3  0  1
PLCP  0  4  3  2  1  0  2  1  0  0
 LPF  0  0  2  1  0  4  3  2  1  0
 ISA  6  3  8  5 10  2  7  4  9  1
 psi  6  7  8  9 10  3  4  5  1  2
  LF  9 10  6  7  8  1  2  3  4  5
 BWT  r  r  b  b  b  $  a  a  a  a
~~~~
