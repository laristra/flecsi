#pragma once

#include <fstream>
#include <iostream>
#include <memory>
#include <regex>

namespace flecsi {
namespace util {
namespace unit {

class test_output_t
{
public:
  static test_output_t & instance() {
    static test_output_t g;
    return g;
  } // instance

  std::ostream & get_stream() {
    return *stream_;
  } // get_stream

  std::string get_buffer() {
    return default_.str();
  } // get_buffer

  void to_file(const std::string & filename) {
    to_file(filename.c_str());
  } // to_file

  void to_file(const char * filename) {
    std::ofstream f(filename);

    if(!f.good()) {
      std::cerr << "Failed to open " << filename << std::endl;
      std::exit(1);
    } // if

    f << default_.rdbuf();
  } // to_file

  bool equal_blessed(const char * filename) {
    std::string testdir_filename(filename);

    // backup rdbuffer, because it will get flushed by to_file
    std::stringstream backup;
    backup << default_.rdbuf();
    backup >> default_.rdbuf();

    // save test output to .current for updates
    size_t lastindex = testdir_filename.find_last_of(".");
    std::string save_output =
      testdir_filename.substr(0, lastindex) + ".current";
    to_file(save_output);

    std::ifstream f(testdir_filename);

    if(!f.good()) {
      std::cerr << "Failed to open " << filename << std::endl;
      std::exit(1);
    } // if

    std::stringstream ss;
    ss << f.rdbuf();

    if(backup.str().compare(ss.str()) == 0) {
      return true;
    } // if

    return false;
  } // equal_blessed

private:
  test_output_t()
    : stream_(new std::ostream(default_.rdbuf())) {} // test_output_t

  std::stringstream default_;
  std::shared_ptr<std::ostream> stream_;

}; // class test_output_t

} // namespace unit
} // namespace util
} // namespace flecsi
