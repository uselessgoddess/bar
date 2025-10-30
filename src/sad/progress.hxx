#pragma once

#include <atomic>
#include <cstdint>
#include <string>
#include <filesystem>

#include <fmt/format.h>
#include <barkeep/barkeep.h>

namespace fs = std::filesystem;

auto format_bytes(uint64_t bytes) -> std::string {
  if (bytes < 1024)
    return fmt::format("{} B", bytes);
  double kb = bytes / 1024.0;
  if (kb < 1024.0)
    return fmt::format("{:.1f} KiB", kb);
  double mb = kb / 1024.0;
  if (mb < 1024.0)
    return fmt::format("{:.2f} MiB", mb);
  double gb = mb / 1024.0;
  return fmt::format("{:.2f} GiB", gb);
}

struct ProgressBar {
  using Progress = barkeep::ProgressBarDisplay<std::atomic<uint64_t>>;
  using Config = barkeep::ProgressBarConfig<uint64_t>;

  ProgressBar(uint64_t total_size) {
    auto config = Config{.total = total_size,
                         .speed = 1.0 / (1024 * 1024),
                         .speed_unit = "MB/s",
                         .style = barkeep::ProgressBarStyle::Rich};
    bar_ = barkeep::ProgressBar(&bytes, config);
  }

  void message(const std::string& message) {
    if (bar_) {
      // bar_->message(message);
    }
  }

  void inc(uint64_t add) { bytes.fetch_add(add, std::memory_order::seq_cst); }

  ~ProgressBar() { bar_->done(); }

 private:
  std::atomic<uint64_t> bytes{0};
  std::shared_ptr<Progress> bar_;
};
