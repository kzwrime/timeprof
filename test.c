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
