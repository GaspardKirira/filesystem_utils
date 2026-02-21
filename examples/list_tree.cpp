#include <filesystem_utils/filesystem_utils.hpp>
#include <iostream>
#include <vector>

namespace fsu = filesystem_utils;

static void print_entries(const std::vector<fsu::fs::path> &v)
{
  for (const auto &p : v)
    std::cout << "  - " << p.string() << "\n";
}

static void run()
{
  const auto root = fsu::fs::temp_directory_path() / "filesystem_utils_examples";
  const auto dir = root / "tree";

  fsu::ensure_dir(dir / "a");
  fsu::ensure_dir(dir / "b" / "c");
  fsu::write_text(dir / "root.txt", "root", true);
  fsu::write_text(dir / "a" / "a.txt", "a", true);
  fsu::write_text(dir / "b" / "c" / "deep.txt", "deep", true);

  std::cout << "Dir: " << dir.string() << "\n\n";

  std::cout << "Direct entries:\n";
  print_entries(fsu::list(dir));

  std::cout << "\nFiles only:\n";
  print_entries(fsu::list_files(dir));

  std::cout << "\nDirs only:\n";
  print_entries(fsu::list_dirs(dir));

  std::cout << "\nRecursive entries:\n";
  print_entries(fsu::list_recursive(dir));
}

int main()
{
  try
  {
    run();
    return 0;
  }
  catch (const std::exception &e)
  {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}
