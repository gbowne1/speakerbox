#include <gtest/gtest.h>
#include "calculator.h"

namespace speakerbox {

TEST(CalculatorTest, Sealed) {
    TSParameters params;
    params.fs = 30.0;
    params.qts = 0.4;
    params.vas = 50.0;
    // ... set others
    Calculator calc;
    EnclosureResult res = calc.calculate(params, EnclosureType::Sealed);
    EXPECT_GT(res.vb, 0.0);
}

}  // namespace speakerbox
