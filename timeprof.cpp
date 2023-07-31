#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include <tuple>
#include <unistd.h>
#include <unordered_map>
#include <utility>
#include <vector>

#include "timeprof.h"

namespace tpf {

void Timeprof::combine(Timeprof_item *current_item) {
  std::unordered_map<std::string, Timeprof_item *> combine_hashmap;

  for (auto item : current_item->sub_regions) {
    auto iter = combine_hashmap.find(item->name);
    if (iter != combine_hashmap.end()) {
      iter->second->seconds += item->seconds;
      iter->second->calltime += item->calltime;
      iter->second->sub_regions.insert(iter->second->sub_regions.end(),
                                       item->sub_regions.begin(),
                                       item->sub_regions.end());
      delete item;
    } else {
      combine_hashmap.insert({item->name, item});
    }
  }

  for (auto item : combine_hashmap) {
    combine(item.second);
  }

  current_item->sub_regions.clear();
  std::transform(combine_hashmap.begin(), combine_hashmap.end(),
                 std::back_inserter(current_item->sub_regions),
                 [](const std::pair<std::string, Timeprof_item *> &p) {
                   return p.second;
                 });

  std::sort(current_item->sub_regions.begin(), current_item->sub_regions.end(),
            cmp_timeprof_item_pointer);
}

std::tuple<int, int, int>
Timeprof::get_max_name_len_depth(const Timeprof_item *current_item,
                                 int begin_depth) {
  int max_name_len = current_item->name.length();
  int max_region_depth = current_item->region_depth;
  int max_name_depth =
      current_item->name.length() + current_item->region_depth - begin_depth;
  for (auto &item : current_item->sub_regions) {
    std::tuple<int, int, int> t = get_max_name_len_depth(item, begin_depth);
    max_name_len = std::max(max_name_len, std::get<0>(t));
    max_region_depth = std::max(max_region_depth, std::get<1>(t));
    max_name_depth = std::max(max_name_depth, std::get<2>(t));
  }
  return std::make_tuple(max_name_len, max_region_depth, max_name_depth);
}

void Timeprof::print_combined(const Timeprof_item *current_item,
                              double all_seconds, double parent_seconds,
                              int offset) {
  printf("%10lf,   %7.3lf %%,   %7.3lf %%,    %7d,   %*s%s\n",
         current_item->seconds, current_item->seconds / parent_seconds * 100,
         current_item->seconds / all_seconds * 100, current_item->calltime,
         offset, "", current_item->name.c_str());
  for (auto &item : current_item->sub_regions) {
    print_combined(item, all_seconds, current_item->seconds, offset + 1);
  }
}

void Timeprof::print(const Timeprof_item *current_item, double all_seconds,
                     double parent_seconds, int offset) {
  printf("%10lf,   %7.3lf %%,   %7.3lf %%,   %*s%s,   %s\n",
         current_item->seconds, current_item->seconds / parent_seconds * 100,
         current_item->seconds / all_seconds * 100, offset, "",
         current_item->name.c_str(), current_item->extra_info.c_str());
  for (auto &item : current_item->sub_regions) {
    print(item, all_seconds, current_item->seconds, offset + 1);
  }
}

void Timeprof::print(const Timeprof_item *current_item, double all_seconds,
                     double parent_seconds, int offset, int max_name_depth) {
  printf("%10lf,   %7.3lf %%,   %7.3lf %%,   %*s%s,  %*s%s\n",
         current_item->seconds, current_item->seconds / parent_seconds * 100,
         current_item->seconds / all_seconds * 100, offset, "",
         current_item->name.c_str(),
         max_name_depth - (int)current_item->name.length() - offset, "",
         current_item->extra_info.c_str());
  for (auto &item : current_item->sub_regions) {
    print(item, all_seconds, current_item->seconds, offset + 1, max_name_depth);
  }
}

void Timeprof::start(std::string name) { start(name, ""); }

void Timeprof::start(std::string name, std::string extra_info) {
  current_region_depth++;

  Timeprof_item *timeprof_item = new Timeprof_item;
  current_item->sub_regions.push_back(timeprof_item);

  timeprof_item->name = name;
  timeprof_item->extra_info = extra_info;
  timeprof_item->seconds = 0;
  timeprof_item->region_depth = current_region_depth;
  timeprof_item->parent_region = current_item;

  timeprof_item->start = std::chrono::high_resolution_clock::now();

  current_item = timeprof_item;
}

void Timeprof::end() {
  current_item->end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed_time =
      current_item->end - current_item->start;
  current_item->seconds = elapsed_time.count();

  current_item = current_item->parent_region;

  current_region_depth--;
}

void Timeprof::print_frame_sorted() {
  Timeprof_item *combined_item = new Timeprof_item;
  combined_item->copy_all(current_item);
  combine(combined_item);
  printf(" Wall time,    Relative,    Absolute,  Call time,   Function\n");
  for (auto &item : combined_item->sub_regions) {
    print_combined(item, item->seconds, item->seconds, 0);
    printf("\n");
  }
  Timeprof_item::delete_all(combined_item);
}

void Timeprof::print_all() {
  printf(" Wall time,    Relative,    Absolute,   Function,   Extra info (may "
         "be empty)\n");
  int max_name_depth = 0;
  for (auto &item : current_item->sub_regions) {
    max_name_depth =
        std::max(max_name_depth,
                 std::get<2>(get_max_name_len_depth(item, item->region_depth)));
  }
  for (auto &item : current_item->sub_regions) {
    print(item, item->seconds, item->seconds, 0, max_name_depth);
    printf("\n");
  }
}

void Timeprof::delete_all() {
  for (auto item : current_item->sub_regions) {
    Timeprof_item::delete_all(item);
  }
}

}; // namespace tpf

static tpf::Timeprof stpf;

extern "C" {
void timeprof_start_(const char *name) { stpf.start(name); }
void timeprof_start_with_info_(const char *name, const char *info) {
  stpf.start(name, info);
}
void timeprof_end_() { stpf.end(); };
void timeprof_print_frame_sorted_() { stpf.print_frame_sorted(); }
void timeprof_print_all_() { stpf.print_all(); }
void timeprof_delete_all_() { stpf.delete_all(); }
}
