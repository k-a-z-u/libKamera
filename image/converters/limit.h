/*
 * limit.h
 *
 *  Created on: Jun 14, 2014
 *      Author: kazu
 */

#ifndef LIMIT_H_
#define LIMIT_H_

#define limit8(x) (uint8_t) ( ((x) < 0) ? (0) : (((x) > 255) ? (255) : (x)) )

#endif /* LIMIT_H_ */
