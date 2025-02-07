#include "dc_json.h"


/* JSON PARSING FUNCTIONS ************************************************************************/
bool jsonParseSkipWhitespace(char c) {
    
    if( c == ' ' 
    ||  c == '\n' 
    ||  c == '\t' 
    ||  c == '\r'
    )   return true;

    return false;
}

const char* jsonParseSkipToValue(const char* jsonString, const char* key) {
    try {

        // Skip past the key and the colon
        const char* start = "";
        if( strstr(jsonString, key)
        )   start = strstr(jsonString, key) + strlen(key);
        
        else
            throw -1;
        
        // skip any whitespace before the value
        while(  jsonParseSkipWhitespace(*start)
        )       start++;  
        
        return start;
    
    } catch (...) { 
        Serial.printf("\n**** jsonParseSkipToValue ERROR : %s **** \n", key); 
        throw -1;   
    }
}

void jsonParseString(const char* jsonString, const char* key, char* destination, size_t maxLen) {
    
    try {
        const char* start = jsonParseSkipToValue(jsonString, key);
        
        start++; // skip string opening quote
        if( start
        ) {
            const char* end = strchr(start, '\"');
            if(end
            ) {
                size_t len = end - start;
                
                if( len >= maxLen
                )   len = maxLen - 1;
                
                strncpy(destination, start, len);
                
                destination[len] = '\0'; // Null-terminate
            }
        }

    } catch (...) { 
        Serial.printf("\n**** jsonParseString ERROR **** \n"); 
        throw -1;   
    }
}

void jsonParseInt(const char* jsonString, const char* key, int& destination) {
    
    try {
        const char* start = jsonParseSkipToValue(jsonString, key);
        
        if( start
        )   destination = atoi(start);

    } catch (...) { 
        Serial.printf("\n**** jsonParseInt ERROR **** \n"); 
        throw -1;   
    }
}

void jsonParseFloat(const char* jsonString, const char* key, float& destination) {
    
    try {
        const char* start = jsonParseSkipToValue(jsonString, key);
        
        if( start
        )   destination = atof(start);
        
    } catch (...) { 
        Serial.printf("\n**** jsonParseFloat ERROR **** \n"); 
        throw -1;   
    }
}

void jsonParseBool(const char* jsonString, const char* key, bool& destination) {
    
    try {
        const char* start = jsonParseSkipToValue(jsonString, key);
        
        if( start
        )   destination = (strncmp(start, "true", 4) == 0); // true if it matches "true"
        
    } catch (...) { 
        Serial.printf("\n**** jsonParseBool ERROR **** \n"); 
        throw -1;   
    }
}


/* JSON SERIALIZING FUNCTIONS ********************************************************************/

void jsonSerializeStart(char* json) {
    
    strcpy(json,"{");
}

void jsonSerializeEnd(char* json) {
    
    size_t len = strlen(json);

    if( len > 1 
    &&  json[len - 1] == ','
    )   json[len - 1] = '}'; 
    
    else 
        strncat(json, "}", strlen(json) - 1); 
    
}

void jsonSerializeBool(char* json, const char* key, bool value) {
    
    strcat(json, key);
    strcat(json, value ? "true" : "false");
    strcat(json, ",");
}

void jsonSerializeInt(char* json, const char* key, int value) {
    
    char buffer[20];                                    // Temporary buffer for integer
    snprintf(buffer, sizeof(buffer), "%d", value);

    strcat(json, key);
    strcat(json, buffer);
    strcat(json, ",");
}

void jsonSerializeFloat(char* json, const char* key, float value) {

    char buffer[20];                                    // Temporary buffer for float
    snprintf(buffer, sizeof(buffer), "%.8f", value);    // Limit to 8 decimal places

    strcat(json, key);
    strcat(json, buffer);
    strcat(json, ",");
}

void jsonSerializeString(char* json, const char* key, const char* value) {
    
    strcat(json, key);
    strcat(json, "\"");
    strcat(json, value);
    strcat(json, "\"");
    strcat(json, ",");
}

