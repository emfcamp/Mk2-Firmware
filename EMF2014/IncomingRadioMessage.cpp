/*
 TiLDA Mk2
 
 Incoming Radio Message
 
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

#include <Sha1.h>

#include "IncomingRadioMessage.h"

IncomingRadioMessage::IncomingRadioMessage(uint32_t aLength,
                                            const byte* aBuffer,
                                            const byte* aHash,
                                            const byte* aSignature,
                                            uint16_t aRID)
    :mLength(aLength), mRID(aRID)
{
    mContent = new byte[mLength];
    memcpy(mContent, aBuffer, mLength);
    memcpy(mHash, aHash, 12);
    memcpy(mSignature, aSignature, 40);
}

IncomingRadioMessage::~IncomingRadioMessage() {
    delete mContent;
}

byte* IncomingRadioMessage::Sha1Result() const {
    // Create SHA1 digest
    Sha1.init();

    char ridHi = mRID >> 8;
    char ridLo = mRID & 0xFF;
    Sha1.print(ridHi);
    Sha1.print(ridLo);

    for (uint32_t i=0; i<mLength; i++) {
        Sha1.print((char)mContent[i]);
    }
    
    return Sha1.result();   
}

byte* IncomingRadioMessage::content() const {
    return mContent;
}

uint32_t IncomingRadioMessage::length() const {
    return mLength;
}

const byte* IncomingRadioMessage::hash() const {
    return mHash;
}

const byte* IncomingRadioMessage::signature() const {
    return mSignature;
}

uint16_t IncomingRadioMessage::rid() const {
    return mRID;
}