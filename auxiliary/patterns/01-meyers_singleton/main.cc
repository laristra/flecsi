#include "meyers_singleton.h"

int main(int argc, char ** argv) {

	// Call the singleton instance and invoke the print method.
	singleton_t::instance().print();

	return 0;
} // main
