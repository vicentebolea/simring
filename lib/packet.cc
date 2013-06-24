#include <simring.hh>

packet::packet (uint64_t p) : point (p) {}	
packet::packet (uint64_t p, uint64_t l, uint64_t u) 
 : point (p), low_b (l), upp_b (u) {}	
packet::packet (const packet& that) : point (that.point) {}

packet& packet::operator= (const packet& that) {
	if (this == &that)
		return *this;

	this->point = that.point;
	return *this;
}

uint64_t packet::get_point (void) { return this->point; }
