/* 
 * File:   string_helper.h
 * Author: Andrew
 *
 * Created on May 16, 2021, 5:29 PM
 */

#ifndef STRING_HELPER_H
#define	STRING_HELPER_H

#include <string>

/**
 * Convert an unsigned int into a string
 * @param val
 * @return pointer to a null terminated string representation of val
 * @note The string returned by this function (or any overloads 
 * of this function) is overridden every time the function is called.
 */
const char* _to_c_string( unsigned int val );

/**
 * Convert an int into a string
 * @param val
 * @return pointer to a null terminated string representation of val
 * @note The string returned by this function (or any overloads 
 * of this function) is overridden every time the function is called.
 */
const char* _to_c_string( int val );

/**
 * Convert an unsigned int into a string
 * @param val
 * @return a string representation of val
 */
std::string _to_string( unsigned int val );

/**
 * Convert an int into a string
 * @param val
 * @return a string representation of val
 */
std::string _to_string( int val );



#endif	/* STRING_HELPER_H */

