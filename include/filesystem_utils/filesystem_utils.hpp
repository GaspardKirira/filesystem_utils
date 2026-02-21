#pragma once

/**
 * @file filesystem_utils.hpp
 * @brief Header-only filesystem helpers built on top of std::filesystem.
 *
 * Goals:
 *  - Safe, predictable primitives for real-world apps
 *  - Atomic writes (best-effort, cross-platform)
 *  - Convenience helpers: read/write, list, copy/remove, sizes
 *  - Header-only, minimal deps, registry-friendly
 *
 * Requirements: C++17 (std::filesystem)
 */

#include <filesystem>
#include <string>
#include <vector>
#include <system_error>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <chrono>

namespace filesystem_utils
{
  namespace fs = std::filesystem;

  /* =========================
     Basics (no-throw wrappers)
  ========================= */

  /**
   * @brief Check whether a path exists (no throw).
   */
  inline bool exists(const fs::path &path) noexcept
  {
    std::error_code ec;
    return fs::exists(path, ec);
  }

  /**
   * @brief Check whether a path is a regular file (no throw).
   */
  inline bool is_file(const fs::path &path) noexcept
  {
    std::error_code ec;
    return fs::is_regular_file(path, ec);
  }

  /**
   * @brief Check whether a path is a directory (no throw).
   */
  inline bool is_directory(const fs::path &path) noexcept
  {
    std::error_code ec;
    return fs::is_directory(path, ec);
  }

  /**
   * @brief Create directory tree if missing.
   *
   * @param dir Directory path.
   * @return true if created or already exists, false on error.
   */
  inline bool ensure_dir(const fs::path &dir) noexcept
  {
    std::error_code ec;
    if (dir.empty())
      return false;

    if (fs::exists(dir, ec))
      return fs::is_directory(dir, ec);

    return fs::create_directories(dir, ec);
  }

  /**
   * @brief Get file size in bytes (no throw).
   *
   * @param path File path
   * @param ok Output flag set to true if size is valid
   * @return size in bytes, or 0 if error
   */
  inline std::uintmax_t file_size_noexcept(const fs::path &path, bool &ok) noexcept
  {
    ok = false;
    std::error_code ec;
    const auto sz = fs::file_size(path, ec);
    if (ec)
      return 0;
    ok = true;
    return sz;
  }

  /* =========================
     Path helpers
  ========================= */

  /**
   * @brief Join two paths safely.
   */
  inline fs::path join(const fs::path &a, const fs::path &b)
  {
    if (a.empty())
      return b;
    if (b.empty())
      return a;
    return a / b;
  }

  /**
   * @brief Best-effort normalize path without touching filesystem.
   *
   * Uses lexically_normal which does not resolve symlinks.
   */
  inline fs::path normalize(const fs::path &p)
  {
    return p.lexically_normal();
  }

  /**
   * @brief Return the filename as UTF-8 string (best-effort).
   */
  inline std::string filename(const fs::path &p)
  {
    return p.filename().string();
  }

  /**
   * @brief Return file extension including dot (e.g. ".txt") or empty.
   */
  inline std::string extension(const fs::path &p)
  {
    return p.extension().string();
  }

  /* =========================
     Read helpers
  ========================= */

  /**
   * @brief Read a whole file as text (UTF-8 best-effort).
   *
   * @throws std::runtime_error on open/read error.
   */
  inline std::string read_text(const fs::path &path)
  {
    std::ifstream in(path, std::ios::in | std::ios::binary);
    if (!in)
      throw std::runtime_error("filesystem_utils::read_text: cannot open file: " + path.string());

    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
  }

  /**
   * @brief Read a whole file as bytes.
   *
   * @throws std::runtime_error on open/read error.
   */
  inline std::vector<std::uint8_t> read_bytes(const fs::path &path)
  {
    std::ifstream in(path, std::ios::in | std::ios::binary);
    if (!in)
      throw std::runtime_error("filesystem_utils::read_bytes: cannot open file: " + path.string());

    in.seekg(0, std::ios::end);
    const std::streamoff size = in.tellg();
    if (size < 0)
      throw std::runtime_error("filesystem_utils::read_bytes: failed to get size: " + path.string());
    in.seekg(0, std::ios::beg);

    std::vector<std::uint8_t> data(static_cast<std::size_t>(size));
    if (size > 0)
      in.read(reinterpret_cast<char *>(data.data()), size);

    if (!in && size > 0)
      throw std::runtime_error("filesystem_utils::read_bytes: read failed: " + path.string());

    return data;
  }

  /* =========================
     Write helpers
  ========================= */

  /**
   * @brief Write text to a file, creating parent directories if needed.
   *
   * @param path Target file path
   * @param text Content
   * @param create_parents If true, parent directories are created
   * @throws std::runtime_error on error
   */
  inline void write_text(const fs::path &path, const std::string &text, bool create_parents = true)
  {
    if (create_parents)
      (void)ensure_dir(path.parent_path());

    std::ofstream out(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!out)
      throw std::runtime_error("filesystem_utils::write_text: cannot open file: " + path.string());

    out.write(text.data(), static_cast<std::streamsize>(text.size()));
    if (!out)
      throw std::runtime_error("filesystem_utils::write_text: write failed: " + path.string());
  }

  /**
   * @brief Write bytes to a file, creating parent directories if needed.
   *
   * @throws std::runtime_error on error
   */
  inline void write_bytes(const fs::path &path,
                          const std::vector<std::uint8_t> &bytes,
                          bool create_parents = true)
  {
    if (create_parents)
      (void)ensure_dir(path.parent_path());

    std::ofstream out(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!out)
      throw std::runtime_error("filesystem_utils::write_bytes: cannot open file: " + path.string());

    if (!bytes.empty())
      out.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));

    if (!out)
      throw std::runtime_error("filesystem_utils::write_bytes: write failed: " + path.string());
  }

  /* =========================
     Atomic write (best-effort)
  ========================= */

  inline std::string _unique_suffix()
  {
    const auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    return std::to_string(static_cast<long long>(now));
  }

  /**
   * @brief Atomically write text to a file (best-effort).
   *
   * Strategy:
   *  - write to temp file in same directory
   *  - rename temp -> target (atomic on most platforms for same filesystem)
   *
   * @throws std::runtime_error on error
   */
  inline void atomic_write_text(const fs::path &path, const std::string &text, bool create_parents = true)
  {
    if (create_parents)
      (void)ensure_dir(path.parent_path());

    const fs::path dir = path.parent_path().empty() ? fs::current_path() : path.parent_path();
    const fs::path tmp = dir / (path.filename().string() + ".tmp." + _unique_suffix());

    try
    {
      write_text(tmp, text, false);

      std::error_code ec;
      fs::rename(tmp, path, ec);
      if (!ec)
        return;

      // Windows: rename may fail if target exists, try remove + rename
      std::error_code ec2;
      fs::remove(path, ec2);
      fs::rename(tmp, path, ec2);
      if (ec2)
        throw std::runtime_error("filesystem_utils::atomic_write_text: rename failed: " + path.string());
    }
    catch (...)
    {
      std::error_code ec;
      fs::remove(tmp, ec);
      throw;
    }
  }

  /**
   * @brief Atomically write bytes to a file (best-effort).
   *
   * @throws std::runtime_error on error
   */
  inline void atomic_write_bytes(const fs::path &path,
                                 const std::vector<std::uint8_t> &bytes,
                                 bool create_parents = true)
  {
    if (create_parents)
      (void)ensure_dir(path.parent_path());

    const fs::path dir = path.parent_path().empty() ? fs::current_path() : path.parent_path();
    const fs::path tmp = dir / (path.filename().string() + ".tmp." + _unique_suffix());

    try
    {
      write_bytes(tmp, bytes, false);

      std::error_code ec;
      fs::rename(tmp, path, ec);
      if (!ec)
        return;

      std::error_code ec2;
      fs::remove(path, ec2);
      fs::rename(tmp, path, ec2);
      if (ec2)
        throw std::runtime_error("filesystem_utils::atomic_write_bytes: rename failed: " + path.string());
    }
    catch (...)
    {
      std::error_code ec;
      fs::remove(tmp, ec);
      throw;
    }
  }

  /* =========================
     Directory listing
  ========================= */

  /**
   * @brief List direct children paths of a directory.
   *
   * @throws std::filesystem::filesystem_error if iterator fails
   */
  inline std::vector<fs::path> list(const fs::path &directory)
  {
    std::vector<fs::path> out;
    for (const auto &entry : fs::directory_iterator(directory))
      out.push_back(entry.path());
    return out;
  }

  /**
   * @brief List paths recursively.
   *
   * @throws std::filesystem::filesystem_error if iterator fails
   */
  inline std::vector<fs::path> list_recursive(const fs::path &directory)
  {
    std::vector<fs::path> out;
    for (const auto &entry : fs::recursive_directory_iterator(directory))
      out.push_back(entry.path());
    return out;
  }

  /**
   * @brief List only files inside a directory (non-recursive).
   */
  inline std::vector<fs::path> list_files(const fs::path &directory)
  {
    std::vector<fs::path> out;
    for (const auto &entry : fs::directory_iterator(directory))
    {
      if (entry.is_regular_file())
        out.push_back(entry.path());
    }
    return out;
  }

  /**
   * @brief List only directories inside a directory (non-recursive).
   */
  inline std::vector<fs::path> list_dirs(const fs::path &directory)
  {
    std::vector<fs::path> out;
    for (const auto &entry : fs::directory_iterator(directory))
    {
      if (entry.is_directory())
        out.push_back(entry.path());
    }
    return out;
  }

  /* =========================
     Copy / remove
  ========================= */

  /**
   * @brief Copy file or directory recursively.
   *
   * @param from Source
   * @param to Destination
   * @param overwrite If true, overwrite existing
   * @return true on success, false on error (no throw)
   */
  inline bool copy_recursive(const fs::path &from, const fs::path &to, bool overwrite = true) noexcept
  {
    std::error_code ec;

    fs::copy_options opt = fs::copy_options::recursive;
    if (overwrite)
      opt |= fs::copy_options::overwrite_existing;
    else
      opt |= fs::copy_options::skip_existing;

    fs::copy(from, to, opt, ec);
    return !ec;
  }

  /**
   * @brief Remove file or directory recursively (no throw).
   *
   * @return number of removed entries.
   */
  inline std::uintmax_t remove_recursive(const fs::path &path) noexcept
  {
    std::error_code ec;
    return fs::remove_all(path, ec);
  }

  /* =========================
     Size formatting
  ========================= */

  /**
   * @brief Format bytes into a human readable string (e.g. "12.4 MB").
   */
  inline std::string human_size(std::uintmax_t bytes)
  {
    static const char *units[] = {"B", "KB", "MB", "GB", "TB", "PB"};
    double v = static_cast<double>(bytes);
    std::size_t i = 0;

    while (v >= 1024.0 && i < (sizeof(units) / sizeof(units[0])) - 1)
    {
      v /= 1024.0;
      ++i;
    }

    std::ostringstream ss;
    ss.setf(std::ios::fixed);
    ss.precision((i == 0) ? 0 : 1);
    ss << v << " " << units[i];
    return ss.str();
  }

} // namespace filesystem_utils
