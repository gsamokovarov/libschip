#include <stdlib.h>
#include "test.h"
#include "test_schip.c"

int main(void) {
  RUN_TEST_SUITE(schip_test_suite);

  return EXIT_SUCCESS;
}
