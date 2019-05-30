#include <flecsi/utils/checksum.h>
#include <flecsi/utils/ftest.h>

const std::size_t N = 100;

int
checksum(int argc, char ** argv) {

  FTEST();

  flecsi::utils::checksum_t cs;

  double array[N];
  for(std::size_t i(0); i < N; ++i) {
    array[i] = double(i);
  } // for

  flecsi::utils::checksum(array, N, cs);

  clog(info) << "checksum: " << cs.strvalue << std::endl;

  ASSERT_STREQ(cs.strvalue, "c0baaf0be574247df89245cd37228336");

  return 0;
}

ftest_register_driver(checksum);
