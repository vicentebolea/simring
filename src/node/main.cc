/*
 * @file This file contains the source code of the application 
 *       which will run in each server 
 *
 *
 */
#include <node.hh>

int main (int argc, const char** argv) {

 Node& node = Node::get_instance ();
 node.setup (argc, argv, "eth0");

 node.run ();
 node.join ();

 return EXIT_SUCCESS;
}
