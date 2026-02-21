#include <filesystem_utils/filesystem_utils.hpp>
#include <iostream>

namespace fsu = filesystem_utils;

static void run()
{
  const auto root = fsu::fs::temp_directory_path() / "filesystem_utils_examples";
  const auto file = root / "hello" / "note.txt";

  fsu::write_text(file, "Hello from filesystem_utils\n", true);

  const auto content = fsu::read_text(file);
  std::cout << "File: " << file.string() << "\n";
  std::cout << "Content:\n"
            << content << "\n";
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
