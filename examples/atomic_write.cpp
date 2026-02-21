#include <filesystem_utils/filesystem_utils.hpp>
#include <iostream>

namespace fsu = filesystem_utils;

static void run()
{
  const auto root = fsu::fs::temp_directory_path() / "filesystem_utils_examples";
  const auto file = root / "atomic" / "state.txt";

  fsu::atomic_write_text(file, "version=1\n", true);
  std::cout << "After v1:\n"
            << fsu::read_text(file) << "\n";

  fsu::atomic_write_text(file, "version=2\n", true);
  std::cout << "After v2:\n"
            << fsu::read_text(file) << "\n";
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
