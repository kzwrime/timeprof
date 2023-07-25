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
  std::vector<Timeprof_item *> sub_regions;
  int region_depth;
  Timeprof_item *parent_region;
  int calltime = 1; // only used in print
  void copy_info(const Timeprof_item *item) {
    this->name = item->name;
    this->extra_info = item->extra_info;
    this->start = item->start;
    this->end = item->end;
    this->seconds = item->seconds;
    this->region_depth = item->region_depth;
    this->parent_region = item->parent_region;
    this->calltime = item->calltime;
  }
  void copy_all(const Timeprof_item *copy_item) {
    copy_info(copy_item);
    for (auto item : copy_item->sub_regions) {
      Timeprof_item *tmp_item = new Timeprof_item;
      tmp_item->copy_all(item);
      sub_regions.push_back(tmp_item);
    }
  }
  static void delete_all(Timeprof_item *item_in) {
    for (auto item : item_in->sub_regions) {
      delete_all(item);
    }
    delete item_in;
  }
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

  static bool cmp_timeprof_item_pointer(Timeprof_item *v1, Timeprof_item *v2) {
    return v1->seconds > v2->seconds;
  }

  void combine(Timeprof_item *current_item);
  std::tuple<int, int, int>
  get_max_name_len_depth(const Timeprof_item *current_item, int begin_depth);
  void print_combined(const Timeprof_item *current_item, double all_seconds,
                      double parent_seconds, int offset);

  void print(const Timeprof_item *current_item, double all_seconds,
             double parent_seconds, int offset);
  void print(const Timeprof_item *current_item, double all_seconds,
             double parent_seconds, int offset, int max_name_len);

public:
  void start(std::string name);

  void start(std::string name, std::string extra_info);

  void end();

  void print_frame_sorted();
  void print_all();

  void delete_all();
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
void timeprof_delete_all_();
#ifdef __cplusplus
}
#endif
