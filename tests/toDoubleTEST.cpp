#include <iostream>
#include <gtest/gtest.h>
#include <uniDQP.h>

long input0[] = { 0, 0, 0};
long input1[] = { 2, 2, 0};
long input2[] = { 1231, 10, 5};
long input3[] = { 27, 0, 10};

TEST(toDouble, positiveHandler){
	ASSERT_EQ(2.0000000001, (double)toDouble(input1));
	ASSERT_EQ(1231.0000000007, (double)toDouble(input2));
	ASSERT_EQ(27.0000000005, (double)toDouble(input3));
}

TEST(toDouble, zeroHandler){
	ASSERT_EQ(0, (double)toDouble(input0));
}

main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
