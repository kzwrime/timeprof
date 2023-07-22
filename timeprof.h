#pragma once

#ifdef __cplusplus

#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <utility>
#include <vector>

namespace tpf {

using time_t = std::chrono::high_resolution_clock::time_point;

class Timeprof_item {
public:
  std::string name;
  std::string extra_info;
  time_t start;
  time_t end;
  double seconds;
  std::vector<Timeprof_item> sub_regions;
  int region_depth;
  Timeprof_item *parent_region;
  int calltime = 1; // only used in print
};

template <typename T1, typename T2> struct PairHash {

  std::size_t operator()(const std::pair<T1, T2> &p) const {
    return std::hash<T1>()(p.first) ^ std::hash<T2>()(p.second);
  }

  std::size_t operator()(const std::pair<int, int> &p) const {
    long long p1 = p.first;
    long long p2 = p.second;
    return std::hash<long long>()((p1 << 32) + p2);
  }
};

struct PairEqual {
  template <typename T1, typename T2>
  bool operator()(const std::pair<T1, T2> &p1,
                  const std::pair<T1, T2> &p2) const {
    return p1.first == p2.first && p1.second == p2.second;
  }
};

class Timeprof {
private:
  // std::unordered_map<std::string, Timeprof_item> hashmap;
  Timeprof_item root_item;
  Timeprof_item *current_item = &root_item;
  // int current_id = 0;
  // std::vector<Timeprof_item> vec_data = {root_item};
  int current_region_depth = 0;
  std::string filename = "timeprof.txt";

  static bool cmp_timeprof_item(Timeprof_item &v1, Timeprof_item &v2) {
    return v1.seconds > v2.seconds;
  }

  std::unordered_map<std::string, Timeprof_item *> combine_hashmap;
  std::vector<Timeprof_item> combine_vector;
  void combine(Timeprof_item &current_item);
  std::tuple<int, int, int> get_max_name_len_depth(Timeprof_item &current_item, int begin_depth);
  void print_combined(Timeprof_item &current_item, double all_seconds,
                      double parent_seconds, int offset);

  void print(Timeprof_item &current_item, double all_seconds,
             double parent_seconds, int offset);
  void print(Timeprof_item &current_item, double all_seconds,
             double parent_seconds, int offset, int max_name_len);

public:
  void start(std::string name);

  void start(std::string name, std::string extra_info);

  void end();

  void print_frame_sorted();
  void print_all();
};

extern tpf::Timeprof stpf;

} // namespace tpf

#endif

#ifdef __cplusplus
extern "C" {
#endif
void timeprof_start_(const char *name);
void timeprof_end_();
void timeprof_print_frame_sorted_();
void timeprof_print_all_();
void timeprof_start_with_info_(const char *name, const char *info);
#ifdef __cplusplus
}
#endif
