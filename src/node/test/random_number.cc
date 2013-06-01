#define _DEBUG

#include <node.hh>
#include <simring.hh>
#include <stdio.h>
#include <string.h>

ssize_t recv_mock (int fd, void* buff, size_t s, int flags) {
	puts ("=======RECV_MOCK=======");
	printf ("FD: %i\nbuff: %s\nsize: %i\nflags: %i\n",
           fd, (char*)buff, (int)s, flags);
	int vect [sizeof(packet)];
	memcpy (buff, vect, sizeof (packet));

	return static_cast<ssize_t> (s);
}

ssize_t send_mock (int fd, const void* buff, size_t s, int flags) {
	puts ("=======SEND_MOCK=======");
	printf ("FD: %i\nbuff: %s\nsize: %i\nflags: %i\n",
           fd, (char*)buff, (int)s, flags);

	return static_cast<ssize_t> (s);
}

//extern "C" {
//}

