#ifndef DC_JSON_H
#define DC_JSON_H

#include <stdio.h>
#include <cstring>
#include <cstdlib> // For atof and atoi

#define btoa(x) ((x)?"true":"false")

#define JSON_OBJECT_SERIALIZED_LENGTH 512
#define JSON_FIELD_STRING_LENGTH 50

/* JSON PARSING FUNCTIONS ************************************************************************/

// Returns true if c is a whitespace character
bool jsonParseSkipWhitespace(char c);

// Returns a pointer to the first character after the 'key'
const char* jsonParseSkipToValue(const char* jsonString, const char* key);

// Helper function for extracting string values
extern void jsonParseString(const char* jsonString, const char* key, char* destination, size_t maxLen);

// Helper function for extracting integer values
extern void jsonParseInt(const char* jsonString, const char* key, int& destination);

// Helper function for extracting float values
extern void jsonParseFloat(const char* jsonString, const char* key, float& destination);

// Helper function for extracting boolean values
extern void jsonParseBool(const char* jsonString, const char* key, bool& destination);


/* JSON SERIALIZING FUNCTIONS ********************************************************************/

// Initialize json output string with '{' 
extern void jsonSerializeStart(char* json);

// Finalize json output string with '}'; removes trailing ',' if present
extern void jsonSerializeEnd(char* json);

// Serialize a boolean value to JSON
extern void jsonSerializeBool(char* json, const char* key, bool value);

// Serialize an integer value to JSON
extern void jsonSerializeInt(char* json, const char* key, int value);

// Serialize a float value to JSON
extern void jsonSerializeFloat(char* json, const char* key, float value);

// Serialize a string value to JSON
extern void jsonSerializeString(char* json, const char* key, const char* value);


#endif /* DC_JSON_H */