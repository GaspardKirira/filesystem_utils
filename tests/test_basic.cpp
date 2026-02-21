#include <filesystem_utils/filesystem_utils.hpp>

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

namespace fsu = filesystem_utils;

static void test_ensure_dir_and_exists()
{
  const auto root = fsu::fs::temp_directory_path() / "filesystem_utils_tests";
  const auto dir = root / "a" / "b" / "c";

  const bool ok = fsu::ensure_dir(dir);
  assert(ok && "ensure_dir should create directories");

  assert(fsu::exists(dir) && "dir should exist after ensure_dir");
  assert(fsu::is_directory(dir) && "dir should be a directory");
}

static void test_write_and_read_text()
{
  const auto root = fsu::fs::temp_directory_path() / "filesystem_utils_tests";
  const auto file = root / "text" / "hello.txt";

  const std::string content = "hello from filesystem_utils\nline2\n";

  fsu::write_text(file, content, true);

  assert(fsu::exists(file) && "text file should exist");
  assert(fsu::is_file(file) && "text file should be a file");

  const auto read_back = fsu::read_text(file);
  assert(read_back == content && "read_text content must match");
}

static void test_atomic_write_text()
{
  const auto root = fsu::fs::temp_directory_path() / "filesystem_utils_tests";
  const auto file = root / "atomic" / "atomic.txt";

  fsu::atomic_write_text(file, "v1\n", true);
  assert(fsu::read_text(file) == "v1\n");

  fsu::atomic_write_text(file, "v2\n", true);
  assert(fsu::read_text(file) == "v2\n");
}

static void test_write_and_read_bytes()
{
  const auto root = fsu::fs::temp_directory_path() / "filesystem_utils_tests";
  const auto file = root / "bytes" / "data.bin";

  const std::vector<std::uint8_t> payload = {0, 1, 2, 3, 4, 250, 251, 252, 253, 254, 255};

  fsu::write_bytes(file, payload, true);
  const auto read_back = fsu::read_bytes(file);

  assert(read_back == payload && "read_bytes must match written bytes");
}

static void test_list_and_human_size()
{
  const auto root = fsu::fs::temp_directory_path() / "filesystem_utils_tests";
  const auto dir = root / "list";

  assert(fsu::ensure_dir(dir));

  fsu::write_text(dir / "a.txt", "a", true);
  fsu::write_text(dir / "b.txt", "bb", true);
  fsu::ensure_dir(dir / "sub");

  const auto entries = fsu::list(dir);
  assert(!entries.empty() && "list should return entries");

  const auto files = fsu::list_files(dir);
  const auto dirs = fsu::list_dirs(dir);

  assert(files.size() >= 2 && "list_files should include a.txt and b.txt");
  assert(dirs.size() >= 1 && "list_dirs should include sub/");

  const auto s1 = fsu::human_size(0);
  const auto s2 = fsu::human_size(1024);
  const auto s3 = fsu::human_size(1024 * 1024);

  assert(!s1.empty() && !s2.empty() && !s3.empty());
}

static void cleanup()
{
  const auto root = fsu::fs::temp_directory_path() / "filesystem_utils_tests";
  (void)fsu::remove_recursive(root);
}

int main()
{
  try
  {
    cleanup();

    test_ensure_dir_and_exists();
    test_write_and_read_text();
    test_atomic_write_text();
    test_write_and_read_bytes();
    test_list_and_human_size();

    cleanup();

    std::cout << "[filesystem_utils] all tests passed\n";
    return 0;
  }
  catch (const std::exception &e)
  {
    std::cerr << "[filesystem_utils] test failed: " << e.what() << "\n";
    cleanup();
    return 1;
  }
}
