#include "unicpp/char_type.h"

#include "gtest/gtest.h"

namespace unicpp {
namespace {

TEST(Decomposition, Basic) {
  const Decomposition* decomp = decomposition(0x1D400);
  ASSERT_NE(decomp, nullptr);

  EXPECT_EQ(decomp->tag, DecompositionTag::Font);
  EXPECT_EQ(decomp->characters, U"A");
}

}  // namespace
}  // namespace unicpp
