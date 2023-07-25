# timeprof

timeprof is a simple C++ library for profiling code regions to measure execution time.

## Usage

To use timeprof:

1. Include `timeprof.h` in your code.
2. Call `timeprof_start_(name)` at the start of the code region you want to profile. `name` is a string label for the region.
3. Call `timeprof_end_()` at the end of the code region.
4. After running your code, call `timeprof_print_frame_sorted_()` to print a summary of times for each profiled region, sorted by total time.

Or call `timeprof_print_all_()` to print details for each profiled region with call counts.

See `test.c` for example usage.

## Features

- Measures wall clock execution time in seconds for code regions.
- Prints summary sorted by total time or detailed output with call counts.
- Nested regions are supported - child times are subtracted from parent times.
- Regions can be labeled with custom names.
- Extra profiling info can be attached to regions.
- Integrates easily with C/C++ code.

The purpose of adding extra info is to count the parameters of some custom functions, so that when analyzing the results, it enables attention to be paid to the relationship between the time and these parameters, such as the length of the string, the shape of the matrix, and the sparsity.

## Implementation

timeprof uses `std::chrono` for timing and stores profile data in a tree structure that matches the nesting of regions.

Output is printed to stdout.

The interface is C compatible so it can be called from C or C++ code. Fortran code can also use it with the help of ISO_C_BINDING or use `// char(0)` to turn fortran character array to c_string.

E.g.

```c
  // Notice the underscore at the end of the function
  void func_(char* str){ /* ... */ }
```

```fortran
  ! Declare the Fortran string variable
  character(len=10) :: fortran_string = "hello"
  ! Convert the Fortran string to a C string
  character(len=10) :: c_string
  c_string = fortran_string // char(0)

  ! Call the C function
  call func(c_string)
```

## Example

Test:

```bash
g++ timeprof.cpp -c -g -Wall && gcc test.c -c -g -Wall && g++ timeprof.o test.o
./a.out
# optional, use valgrind to detect memory leak, run the command and see valReport
valgrind --log-file=valReport --leak-check=full --show-reachable=yes --leak-resolution=low ./a.out
```

Example result.

```
  func_a
    func_b
      func_c
        func_d
    func_b
      func_c
        func_d
    func_c
      func_d
  func_b
    func_c
      func_d

The following part is the output by timeprof.

 Wall time,    Relative,    Absolute,  Call time,   Function
  8.003162,   100.000 %,   100.000 %,          1,   func_a
  6.002196,    74.998 %,    74.998 %,          1,    func_b
  4.001102,    66.661 %,    49.994 %,          1,     func_c
  2.000555,    50.000 %,    24.997 %,          1,      func_d
  2.000731,    24.999 %,    24.999 %,          1,    func_c
  1.000319,    49.998 %,    12.499 %,          1,     func_d

  3.000807,   100.000 %,   100.000 %,          1,   func_b
  2.000561,    66.667 %,    66.667 %,          1,    func_c
  1.000354,    50.004 %,    33.336 %,          1,     func_d

-------------------

 Wall time,    Relative,    Absolute,   Function,   Extra info (may be empty)
  8.003162,   100.000 %,   100.000 %,   func_a,     
  3.000812,    37.495 %,    37.495 %,    func_b,    some other info
  2.000425,    66.663 %,    24.995 %,     func_c,   
  1.000239,    50.001 %,    12.498 %,      func_d,  len=0
  3.001384,    37.502 %,    37.502 %,    func_b,    some other info
  2.000677,    66.658 %,    24.999 %,     func_c,   
  1.000317,    49.999 %,    12.499 %,      func_d,  len=1
  2.000731,    24.999 %,    24.999 %,    func_c,    
  1.000319,    49.998 %,    12.499 %,     func_d,   len=2

  3.000807,   100.000 %,   100.000 %,   func_b,     some other info
  2.000561,    66.667 %,    66.667 %,    func_c,    
  1.000354,    50.004 %,    33.336 %,     func_d,   len=3

```

Example code.

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "timeprof.h"

static int depth = 0;
void func_d(int len) {
  char tmp_str[100];
  sprintf(tmp_str, "len=%d", len);
  timeprof_start_with_info_(__func__, tmp_str);
  depth++;
  printf("%*s%s\n", depth * 2, "", __func__);
  depth--;
  sleep(1);
  timeprof_end_();
}
void func_c() {
  timeprof_start_(__func__);
  static int count = 0;
  depth++;
  printf("%*s%s\n", depth * 2, "", __func__);
  func_d(count++);
  depth--;
  sleep(1);
  timeprof_end_();
}
void func_b() {
  timeprof_start_with_info_(__func__, "some other info");
  depth++;
  printf("%*s%s\n", depth * 2, "", __func__);
  func_c();
  depth--;
  sleep(1);
  timeprof_end_();
}
void func_a() {
  timeprof_start_(__func__);
  depth++;
  printf("%*s%s\n", depth * 2, "", __func__);
  func_b();
  func_b();
  func_c();
  depth--;
  timeprof_end_();
}
int main() {
  func_a();
  func_b();

  printf("\n\n");

  timeprof_print_frame_sorted_();
  printf("\n\n-------------------\n\n");

  timeprof_print_all_();

  timeprof_delete_all_();
}

```

## Others

The Usage, Features and Implementation part of this README is auto generated by claude 2.