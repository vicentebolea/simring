
#include <node.hh>

int connect_mock (int sock, const struct sockaddr* a, socklen_t l) {
 printf ("CONNECT called\n");
 return 0;
}

void parse_args (int argc, char** argv) {
	strcpy (host_str, "HOST");
	strcpy (peer_right, "right_server");
	strcpy (peer_left, "left_server");
	strcpy (data_file, "ToBeDetermined");
}
