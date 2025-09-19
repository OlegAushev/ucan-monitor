#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>

class CsvLogger {
private:
  std::filesystem::path log_dir_;
  std::filesystem::path current_path_;

  std::uintmax_t current_filesize_{0};
  uintmax_t max_filesize_;

  int writes_since_flush_{0};
  int flush_after_writes_{100};

  std::chrono::seconds cleanup_after_;

  std::ofstream out_;
public:
  CsvLogger(
      std::string dir,
      uintmax_t max_filesize,
      std::chrono::seconds cleanup_after)
      : log_dir_(dir),
        max_filesize_(max_filesize),
        cleanup_after_(cleanup_after) {
    std::filesystem::create_directory(dir);
    cleanup_old_files();
    rotate();
  }

  ~CsvLogger() { close(); }

  void write(std::string_view text, float value) {
    // Rotate if needed before writing next line
    // to keep files <= max_filesize_
    if (current_filesize_ >= max_filesize_) {
      rotate();
    }

    // Compose CSV line
    // ISO 8601 local timestamp with seconds and milliseconds
    // (e.g., 2025-09-19T16:42:07.123)
    std::string ts = now_local_iso8601_ms();
    out_ << ts << ',' << text << ',' << value << '\n';

    // Track bytes written (approx: length of line we just appended)
    current_filesize_ += ts.size() + 1 + text.size() + 1 /*comma*/ +
                         estimate_float_len(value) + +1 /*newline*/;
  }

private:
  void close() {
    if (out_.is_open()) {
      out_.flush();
      out_.close();
    }
  }

  void rotate() {
    close();

    // Create new file path with timestamp
    // Ensure uniqueness if rotated within same second
    int attempt{0};
    do {
      std::ostringstream name;
      name << "data_" << date_time_for_filename();
      if (attempt > 0) {
        name << '_' << attempt;
      }
      name << ".csv";
      current_path_ = std::filesystem::path(log_dir_) / name.str();
      attempt++;
    } while (std::filesystem::exists(current_path_));

    out_.open(current_path_, std::ios::out | std::ios::trunc);
    if (!out_) {
      throw std::runtime_error(
          "Failed to open log file: " + current_path_.string());
    }
    current_filesize_ = 0;
    write_header();
  }

  static std::string date_time_for_filename() {
    // For unique filenames: YYYYMMDD_HHMMSS
    using namespace std::chrono;
    auto now = system_clock::now();
    std::time_t t = system_clock::to_time_t(now);
    std::tm tm{};
    localtime_r(&t, &tm);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
    return oss.str();
  }

  static std::string now_local_iso8601_ms() {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    std::time_t t = system_clock::to_time_t(now);
    std::tm tm{};
    localtime_r(&t, &tm);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S") << '.' << std::setw(3)
        << std::setfill('0') << ms.count();
    return oss.str();
  }

  static std::size_t estimate_float_len([[maybe_unused]] float v) {
    // Conservative estimate: up to e.g. "-123456.123456"
    // To be precise, we could stringify, but this is faster.
    return 20;
  }

  void write_header() {
    std::string const header{"timestamp,object,value\n"};
    out_ << header;
    current_filesize_ += header.size();
  }

  static std::chrono::system_clock::time_point
  to_sys_time(std::filesystem::file_time_type ftime) {
    // Convert file_time_type -> system_clock::time_point portably
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(
        ftime - std::filesystem::file_time_type::clock::now() +
        system_clock::now());
    return sctp;
  }

  void cleanup_old_files() {
    using namespace std::chrono;
    auto const now = system_clock::now();
    auto const cutoff = now - cleanup_after_;

    for (auto const& entry : std::filesystem::directory_iterator(log_dir_)) {
      if (!entry.is_regular_file())
        continue;
      auto const path = entry.path();
      if (path.extension() != ".csv")
        continue;

      std::error_code ec;
      auto ftime = std::filesystem::last_write_time(path, ec);
      if (ec)
        continue;

      if (to_sys_time(ftime) < cutoff) {
        // Avoid deleting a file we currently have open (paranoia)
        if (current_path_.empty() || path != current_path_) {
          std::filesystem::remove(path, ec);
        }
      }
    }
  }
};
