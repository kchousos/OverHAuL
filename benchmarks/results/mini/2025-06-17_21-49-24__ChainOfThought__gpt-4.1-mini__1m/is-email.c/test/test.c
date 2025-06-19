
#include "../src/is-email.h"
#include "describe.h"

int main(int argc, char **argv) {
  describe("is-email", {
    it("should recognize valid email addresses", {
      assert(0 == is_email("foo@bar.com"));
      assert(0 == is_email("f.oo@bar.com"));
      assert(0 == is_email("fo+o@bar.com"));
      assert(0 == is_email("foo3@bar.com"));
      assert(0 == is_email("foo@ba-r.com"));
      assert(0 == is_email("a@b.com"));
    });

    it("should recognize invalid email addresses", {
      assert(-1 == is_email("not even close"));
      assert(-1 == is_email("fairly@close"));
      assert(-1 == is_email("foo@.com"));
      assert(-1 == is_email("foo+bar.com"));
      assert(-1 == is_email("@foobar.com"));
      assert(-1 == is_email("foo@bar."));
      assert(-1 == is_email("foo@bar.a"));
      assert(-1 == is_email("whatever.@com"));
      assert(-1 == is_email("a@b.c"));
    });
  });
  return assert_failures();
}
