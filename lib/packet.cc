#include <simring.hh>

packet::packet (int p) : point (p) {}	
packet::packet (const packet& that) : point (that.point) {}

packet& packet::operator= (const packet& that) {
	if (this == &that)
		return *this;

	this->point = that.point;
	return *this;
}

int packet::get_point (void) { return this->point; }
