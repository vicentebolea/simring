#include <iostream>
#include <uniDQP.h>
#include <gtest/gtest.h>

class toArray: public testing::Test{
protected:

	char line0;
	char line1;
	char line2;
	char line3;

	long res0[];
	long res1[];
	long res2[];
	long res3[];
	long* t1,t2,t3;	

	bool isEqual(long* a, long* b)
	{
		if (a[0] == b[0] && a[1] == b[1] && a[2] == b[2] )
			return true;
		return false;
	}

	virtual void SetUp(){
		line0[]={""};
		line1[]={"20.30.0"};
		line2[]={"12.0.0"};
		line3[]={"120.100.10"};

		res0[] = {0,0,0};
		res1[] = {20,30,0};
		res2[] = {12,0,0};
		res3[] = {120,100,10};
	}

	void mainTest(){
		t1 = packet::toArray(line1));
		t2 = packet::toArray(line2));
		t3 = packet::toArray(line3));
		ASSERT_TRUE(isEqual(res1,t1);
		ASSERT_TRUE(isEqual(res2,t2);
		ASSERT_TRUE(isEqual(res3,t3);
	}

	virtual void TearDown(){
		delete[] t1;
		delete[] t2;
		delete[] t3;
	}
};

TEST(toArray, postiveHandler){
	mainTest();
}

int main(int argc, char** argv){
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
