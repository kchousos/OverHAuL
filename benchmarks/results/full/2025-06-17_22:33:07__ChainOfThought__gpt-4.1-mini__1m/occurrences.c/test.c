
#include <stdlib.h>
#include "describe/describe.h"
#include "occurrences.h"

int
main(void) {
  describe("occurrences") {
    it("should return -1 when given bad inputs") {
      assert(-1 == (int) occurrences(NULL, NULL));
      assert(-1 == (int) occurrences("foo", NULL));
      assert(-1 == (int) occurrences(NULL, "bar"));
    }

    it("should count the number of occurrences in haystack") {
      assert(2 == (int) occurrences("l", "hello"));
      assert(1 == (int) occurrences("foo", "foo"));
    }

    it("should handle anti-cases") {
      assert(0 == (int) occurrences("foo", "bar"));
    }
  }

  return assert_failures();
}
