#define _DEBUG
#include <node.hh>

static int c = 0;
int connect_mock (int sock, const struct sockaddr* a, socklen_t l) {
 printf ("CONNECT called\n");
 return 0;
}

void parse_args (int argc, const char** argv) {
	strcpy (host_str, "HOST");
	strcpy (peer_right, "right_server");
	strcpy (peer_left, "left_server");
	strcpy (data_file, "ToBeDetermined");
}

void recv_msg (int fd, char* in) {
 if (c++ < 5) {
	 strcpy (in, "QUERY");
 } else {
	 strcpy (in, "INFO");
 }
}
