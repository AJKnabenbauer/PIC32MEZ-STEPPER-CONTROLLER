#include "string_helper.h"

// global shared buffer for c string functions
char g_c_buffer[33] = "";



const char* _to_c_string( unsigned int val )
{
        snprintf( g_c_buffer, sizeof (g_c_buffer), "%u", val );
        return g_c_buffer;
}



const char* _to_c_string( int val )
{
        snprintf( g_c_buffer, sizeof (g_c_buffer), "%d", val );
        return g_c_buffer;
}



std::string _to_string( unsigned int val )
{
        char buffer[33] = "";
        snprintf( buffer, sizeof (buffer), "%u", val );
        return std::string( buffer );
}



std::string _to_string( int val )
{
        char buffer[33] = "";
        snprintf( buffer, sizeof (buffer), "%d", val );
        return std::string( buffer );
}



