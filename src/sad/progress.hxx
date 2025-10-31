#pragma once

#include <atomic>
#include <cstdint>
#include <string>
#include <filesystem>

#include <fmt/format.h>
#include <barkeep/barkeep.h>

namespace fs = std::filesystem;

using order = std::memory_order;
using Style = barkeep::ProgressBarStyle;

auto mb(uint64_t bytes) -> float {
  return bytes / (1024.0 * 1024.0);
}

struct ProgressBar {
  ProgressBar(uint64_t files, uint64_t bytes) {
    auto bar = barkeep::ProgressBar(&bytes_, {
                                                 .total = mb(bytes),
                                                 .format = "{speed:.1f} MB/s {bar}",
                                                 .speed = 0.0,
                                                 .speed_unit = "MB/s",
                                                 .style = Style::Rich,
                                                 .show = false,
                                             });

    auto format = fmt::format("{{value}}/{}", files);
    auto counter = barkeep::Counter(&files_, {
                                                 .format = format,
                                                 .show = false,
                                             });
    bar_ = bar | counter;
    bar_->show();
  }

  void inc(uint64_t bytes) {
    files_.fetch_add(1, order::seq_cst);
    bytes_.fetch_add(mb(bytes), order::seq_cst);
  }

  ~ProgressBar() { bar_->done(); }

 private:
  std::atomic<float> bytes_{0};
  std::atomic<uint64_t> files_{0};

  std::shared_ptr<barkeep::CompositeDisplay> bar_;
};
