#include <uniDQP.h>
using namespace std;


int main (int argc, char** argv)
{
	char	i1[] = "1 0 1000 10";
	char	i2[] = "2 0 1000 10";
	char	i3[] = "3 0 1000 10";
	char	i4[] = "5 0 1000 10";
	char	i5[] = "6 0 1000 10";

	packet p1 (i1);
	packet p2 (i2);
	packet p3 (i3);
	packet p4 (i4);
	packet p5 (i5);

	SETcache a(100);

	int count[2] = {0};
	a.match(&p1, count);

	a.match(&p2, count);
	a.match(&p3, count);
	a.match(&p4, count);
	a.match(&p5, count);

	cout<< "Count: "<<count[0]<< endl;	

	a.match(&p1, count);
	a.match(&p2, count);
	a.match(&p3, count);
	a.match(&p4, count);
	a.match(&p5, count);

	cout<< "Count: "<<count[0]<< endl;	
}
