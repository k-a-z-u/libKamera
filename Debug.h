#ifndef K_DEBUG_H
#define K_DEBUG_H

/** helper for debug output */

#include <stdlib.h>
#include <iostream>

#define debug(src, msg)				std::cout << "[" << src << "] " << msg << std::endl;
//#define debug(msg)				std::cout << msg << std::endl;
#define debugBool(src, msg, val)	std::cout << "[" << src << "] " << msg << std::string( (val) ? "true" : "false" ) << std::endl;


// print an error and exit
//#define error(src, msg)				std::cerr << "[" << src << "] " << msg << std::endl; exit(-1);

#endif
