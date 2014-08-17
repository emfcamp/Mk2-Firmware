/*
 TiLDA Mk2
 
 Incoming Radio Message

 This is a consumer task that checks messages received by the radio task for validity

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

#pragma once

#include <Arduino.h>

class IncomingRadioMessage {
public:
    IncomingRadioMessage(uint32_t aLength,
                            const byte* aBuffer,
                            const byte* aHash,
                            const byte* aSignature,
                            uint16_t aRID);
    ~IncomingRadioMessage();

    byte* Sha1Result() const;

    byte* content() const;
    uint32_t length() const;
    const byte* hash() const;
    const byte* signature() const;
    uint16_t rid() const;

private:
    IncomingRadioMessage(){;}
    IncomingRadioMessage(const IncomingRadioMessage&){;}

private:
    byte* mContent;
    uint32_t mLength;
    byte mHash[12];
    byte mSignature[40];
    uint16_t mRID;
};
