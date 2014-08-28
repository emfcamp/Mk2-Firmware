/*
 TiLDA Mk2

 Utils

 The MIT License (MIT)

 Copyright (c) 2014 Electromagnetic Field LTD

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include "Utils.h"

uint16_t Utils::bytesToInt(byte b1, byte b2) {
    int result = 0;
    result = (result << 8) + b1;
    result = (result << 8) + b2;
    return result;
}

uint32_t Utils::bytesToInt(byte b1, byte b2, byte b3, byte b4) {
    int result = 0;
    result = (result << 8) + b1;
    result = (result << 8) + b2;
    result = (result << 8) + b3;
    result = (result << 8) + b4;
    return result;
}

String Utils::intToHex(uint8_t input) {
    return String("0123456789abcdef"[input>>4]) + String("0123456789abcdef"[input&0xf]);
}

bool Utils::getBoolean(PackReader& reader) {
    reader.next();
    return reader.getBoolean();
}

tp_integer_t Utils::getInteger(PackReader& reader) {
    reader.next();
    return reader.getInteger();
}

char* Utils::getString(PackReader& reader) {
    reader.next();
    char* string = new char[reader.contentLength() + 1];
    reader.getString(string, reader.contentLength() + 1);
    return string;
}

char* Utils::wordWrap(char* buffer, const char* string, const uint8_t line_width, const uint8_t max_lines) {
    int i = 0;
    int k, counter;
    int lines = 0;
 
    while(i < strlen( string ) && lines < max_lines) 
    {
        // copy string until the end of the line is reached
        for ( counter = 1; counter <= line_width; counter++ ) 
        {
            // check if end of string reached
            if ( i == strlen( string ) ) 
            {
                buffer[ i ] = 0;
                return buffer;
            }
            buffer[ i ] = string[ i ];
            // check for newlines embedded in the original input 
            // and reset the index
            if ( buffer[ i ] == '\n' )
            {
                lines++;
                counter = 1; 
            }
            i++;
        }
        // check for whitespace
        if ( isspace( string[ i ] ) ) 
        {
            lines++;
            buffer[i] = '\n';
            i++;
        } 
        else
        {
            // check for nearest whitespace back in string
            for ( k = i; k > 0; k--) 
            {
                if ( isspace( string[ k ] ) ) 
                {
                    lines++;
                    buffer[ k ] = '\n';
                    // set string index back to character after this one
                    i = k + 1;
                    break;
                }
            }
        }
    }
    buffer[ i ] = 0;
 
    return buffer;
}

