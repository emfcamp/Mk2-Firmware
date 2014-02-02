/* Arduino SdFat Library
 * Copyright (C) 2013 by William Greiman
 *
 * This file is part of the Arduino SdFat Library
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Arduino SdFat Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include <SdFat.h>
// Use Stimmer div/mod 10 on avr
#ifdef __AVR__
#define USE_STIMMER
#endif  // __AVR__
//------------------------------------------------------------------------------
// Stimmer div/mod 10 for AVR
// this code fragment works out i/10 and i%10 by calculating
// i*(51/256)*(256/255)/2 == i*51/510 == i/10
// by "j.k" I mean 32.8 fixed point, j is integer part, k is fractional part
// j.k = ((j+1.0)*51.0)/256.0
// (we add 1 because we will be using the floor of the result later)
// divmod10_asm16 and divmod10_asm32 are public domain code by Stimmer.
// http://forum.arduino.cc/index.php?topic=167414.msg1293679#msg1293679
#define divmod10_asm16(in32, mod8, tmp8)		\
asm volatile (					\
      " ldi %2,51     \n\t"			\
      " mul %A0,%2    \n\t"			\
      " clr %A0       \n\t"			\
      " add r0,%2     \n\t"			\
      " adc %A0,r1    \n\t"			\
      " mov %1,r0     \n\t"			\
      " mul %B0,%2    \n\t"			\
      " clr %B0       \n\t"			\
      " add %A0,r0    \n\t"			\
      " adc %B0,r1    \n\t"			\
      " clr r1        \n\t"  		\
      " add %1,%A0    \n\t"			\
      " adc %A0,%B0   \n\t"			\
      " adc %B0,r1   \n\t"			\
      " add %1,%B0    \n\t"			\
      " adc %A0,r1   \n\t"			\
      " adc %B0,r1    \n\t"			\
      " lsr %B0       \n\t"			\
      " ror %A0       \n\t"			\
      " ror %1        \n\t"   	\
      " ldi %2,10     \n\t"			\
      " mul %1,%2     \n\t"			\
      " mov %1,r1     \n\t"			\
      " clr r1        \n\t"			\
      :"+r"(in32),"=d"(mod8),"=d"(tmp8) : : "r0")

#define divmod10_asm32(in32, mod8, tmp8)		\
asm volatile (					\
      " ldi %2,51     \n\t"			\
      " mul %A0,%2    \n\t"			\
      " clr %A0       \n\t"			\
      " add r0,%2     \n\t"			\
      " adc %A0,r1    \n\t"			\
      " mov %1,r0     \n\t"			\
      " mul %B0,%2    \n\t"			\
      " clr %B0       \n\t"			\
      " add %A0,r0    \n\t"			\
      " adc %B0,r1    \n\t"			\
      " mul %C0,%2    \n\t"			\
      " clr %C0       \n\t"			\
      " add %B0,r0    \n\t"			\
      " adc %C0,r1    \n\t"			\
      " mul %D0,%2    \n\t"			\
      " clr %D0       \n\t"			\
      " add %C0,r0    \n\t"			\
      " adc %D0,r1    \n\t"			\
      " clr r1        \n\t"  		\
      " add %1,%A0    \n\t"			\
      " adc %A0,%B0   \n\t"			\
      " adc %B0,%C0   \n\t"			\
      " adc %C0,%D0   \n\t"			\
      " adc %D0,r1    \n\t"			\
      " add %1,%B0    \n\t"			\
      " adc %A0,%C0   \n\t"			\
      " adc %B0,%D0   \n\t"			\
      " adc %C0,r1    \n\t"			\
      " adc %D0,r1    \n\t"			\
      " add %1,%D0    \n\t"			\
      " adc %A0,r1    \n\t"			\
      " adc %B0,r1    \n\t"			\
      " adc %C0,r1    \n\t"			\
      " adc %D0,r1    \n\t"			\
      " lsr %D0       \n\t"			\
      " ror %C0       \n\t"			\
      " ror %B0       \n\t"			\
      " ror %A0       \n\t"			\
      " ror %1        \n\t"   	\
      " ldi %2,10     \n\t"			\
      " mul %1,%2     \n\t"			\
      " mov %1,r1     \n\t"			\
      " clr r1        \n\t"			\
      :"+r"(in32),"=d"(mod8),"=d"(tmp8) : : "r0")
//------------------------------------------------------------------------------
// Format 16-bit unsigned
static uint8_t* fmtDec(uint16_t n, uint8_t* p) {
  while (n > 9) {
#ifdef USE_STIMMER
    uint8_t tmp8, r;
    divmod10_asm16(n, r, tmp8);
#else  // USE_STIMMER
    uint16_t t = n;
    n = ((n >> 1) + n) >> 1;
    n = n + (n >> 4);
    n = n + (n >> 8);
    //n = n + (n >> 16);  // no code for 16-bit n
    n = n >> 3;
    uint8_t r = t - (((n << 2) + n) << 1);
    if (r > 9) {
      n++;
      r -= 10;
    }
#endif  // USE_STIMMER
    *--p = r + '0';
  }
  *--p = n + '0';
  return p;
}
//------------------------------------------------------------------------------
// format 32-bit unsigned
static uint8_t* fmtDec(uint32_t n, uint8_t* p) {
  while (n >> 16) {
#ifdef USE_STIMMER
    uint8_t tmp8, r;
    divmod10_asm32(n, r, tmp8);
#else  //  USE_STIMMER
    uint32_t t = n;
    n = ((n >> 1) + n) >> 1;
    n = n + (n >> 4);
    n = n + (n >> 8);
    n = n + (n >> 16);
    n = n >> 3;
    uint8_t r = t - (((n << 2) + n) << 1);
    if (r > 9) {
      n++;
      r -= 10;
    }
#endif  // USE_STIMMER
    *--p = r + '0';
  }
  return fmtDec((uint16_t)n, p);
}
//------------------------------------------------------------------------------
// Format field terminator
static uint8_t* fmtTerm(char term, uint8_t* p) {
  if (term) {
    *--p = term;
    if (term == '\n') *--p = '\r';
  }
  return p;
}
//------------------------------------------------------------------------------
/** Template for SdBaseFile::printField() */
template <typename Type>
static int printFieldT(SdBaseFile* file, Type value, char term, bool neg) {
  uint8_t buf[3*sizeof(Type) + 3];
  uint8_t* ptr = fmtTerm(term, buf + sizeof(buf));
  ptr = fmtDec(value, ptr);
  if (neg) *--ptr = '-';
  return file->write(ptr, buf + sizeof(buf) - ptr);
}
//------------------------------------------------------------------------------
/** List directory contents to stdOut.
 *
 * \param[in] flags The inclusive OR of
 *
 * LS_DATE - %Print file modification date
 *
 * LS_SIZE - %Print file size.
 *
 * LS_R - Recursive list of subdirectories.
 */
void SdBaseFile::ls(uint8_t flags) {
  ls(SdFat::stdOut(), flags, 0);
}
//------------------------------------------------------------------------------
/** List directory contents.
 *
 * \param[in] pr Print stream for list.
 *
 * \param[in] flags The inclusive OR of
 *
 * LS_DATE - %Print file modification date
 *
 * LS_SIZE - %Print file size.
 *
 * LS_R - Recursive list of subdirectories.
 *
 * \param[in] indent Amount of space before file name. Used for recursive
 * list to indicate subdirectory level.
 */
void SdBaseFile::ls(Print* pr, uint8_t flags, uint8_t indent) {
  rewind();
  int8_t status;
  while ((status = lsPrintNext(pr, flags, indent))) {
    if (status > 1 && (flags & LS_R)) {
      uint16_t index = curPosition()/32 - 1;
      SdBaseFile s;
      if (s.open(this, index, O_READ)) s.ls(pr, flags, indent + 2);
      seekSet(32 * (index + 1));
    }
  }
}
//------------------------------------------------------------------------------
// saves 32 bytes on stack for ls recursion
// return 0 - EOF, 1 - normal file, or 2 - directory
int8_t SdBaseFile::lsPrintNext(Print *pr, uint8_t flags, uint8_t indent) {
  dir_t dir;
  uint8_t w = 0;

  while (1) {
    if (read(&dir, sizeof(dir)) != sizeof(dir)) return 0;
    if (dir.name[0] == DIR_NAME_FREE) return 0;

    // skip deleted entry and entries for . and  ..
    if (dir.name[0] != DIR_NAME_DELETED && dir.name[0] != '.'
      && DIR_IS_FILE_OR_SUBDIR(&dir)) break;
  }
  // indent for dir level
  for (uint8_t i = 0; i < indent; i++) pr->write(' ');

  // print name
  for (uint8_t i = 0; i < 11; i++) {
    if (dir.name[i] == ' ')continue;
    if (i == 8) {
      pr->write('.');
      w++;
    }
    pr->write(dir.name[i]);
    w++;
  }
  if (DIR_IS_SUBDIR(&dir)) {
    pr->write('/');
    w++;
  }
  if (flags & (LS_DATE | LS_SIZE)) {
    while (w++ < 14) pr->write(' ');
  }
  // print modify date/time if requested
  if (flags & LS_DATE) {
    pr->write(' ');
    printFatDate(pr, dir.lastWriteDate);
    pr->write(' ');
    printFatTime(pr, dir.lastWriteTime);
  }
  // print size if requested
  if (!DIR_IS_SUBDIR(&dir) && (flags & LS_SIZE)) {
    pr->write(' ');
    pr->print(dir.fileSize);
  }
  pr->println();
  return DIR_IS_FILE(&dir) ? 1 : 2;
}
//------------------------------------------------------------------------------
// print uint8_t with width 2
static void print2u(Print* pr, uint8_t v) {
  if (v < 10) pr->write('0');
  pr->print(v, DEC);
}
//------------------------------------------------------------------------------
/** Print a file's creation date and time
 *
 * \param[in] pr Print stream for output.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool SdBaseFile::printCreateDateTime(Print* pr) {
  dir_t dir;
  if (!dirEntry(&dir)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  printFatDate(pr, dir.creationDate);
  pr->write(' ');
  printFatTime(pr, dir.creationTime);
  return true;

 fail:
  return false;
}
//------------------------------------------------------------------------------
/** %Print a directory date field to stdOut.
 *
 *  Format is yyyy-mm-dd.
 *
 * \param[in] fatDate The date field from a directory entry.
 */
void SdBaseFile::printFatDate(uint16_t fatDate) {
  printFatDate(SdFat::stdOut(), fatDate);
}
//------------------------------------------------------------------------------
/** %Print a directory date field.
 *
 *  Format is yyyy-mm-dd.
 *
 * \param[in] pr Print stream for output.
 * \param[in] fatDate The date field from a directory entry.
 */
void SdBaseFile::printFatDate(Print* pr, uint16_t fatDate) {
  pr->print(FAT_YEAR(fatDate));
  pr->write('-');
  print2u(pr, FAT_MONTH(fatDate));
  pr->write('-');
  print2u(pr, FAT_DAY(fatDate));
}
//------------------------------------------------------------------------------
/** %Print a directory time field to stdOut.
 *
 * Format is hh:mm:ss.
 *
 * \param[in] fatTime The time field from a directory entry.
 */
void SdBaseFile::printFatTime(uint16_t fatTime) {
  printFatTime(SdFat::stdOut(), fatTime);
}
//------------------------------------------------------------------------------
/** %Print a directory time field.
 *
 * Format is hh:mm:ss.
 *
 * \param[in] pr Print stream for output.
 * \param[in] fatTime The time field from a directory entry.
 */
void SdBaseFile::printFatTime(Print* pr, uint16_t fatTime) {
  print2u(pr, FAT_HOUR(fatTime));
  pr->write(':');
  print2u(pr, FAT_MINUTE(fatTime));
  pr->write(':');
  print2u(pr, FAT_SECOND(fatTime));
}
//------------------------------------------------------------------------------
/** Print a number followed by a field terminator.
 * \param[in] value The number to be printed.
 * \param[in] term The field terminator.  Use '\\n' for CR LF.
 * \param[in] prec Number of digits after decimal point.
 * \return The number of bytes written or -1 if an error occurs.
 */
int SdBaseFile::printField(double value, char term, uint8_t prec) {
  bool neg = false;
  uint8_t buf[24];
  uint8_t* ptr = fmtTerm(term, buf + sizeof(buf));
  if (value < 0) {
    value = -value;
    neg = true;
  }
  // check for NaN INF OVF
  if (isnan(value)) {
    *--ptr = 'N';
    *--ptr = 'a';
    *--ptr = 'N';
  } else if (isinf(value)) {
    *--ptr = 'F';
    *--ptr = 'N';
    *--ptr = 'I';
  } else if (value > 4294967040.0) {
    *--ptr = 'F';
    *--ptr = 'V';
    *--ptr = 'O';
  } else {
    if (prec > 9) prec = 9;
    uint32_t s = 1;
    for ( uint8_t i = 0; i < prec; i++) {
      s = ((s << 2) + s) << 1;
    }
    // round value
    value += 0.5/s;
    uint32_t whole = value;
    if (prec) {
      uint8_t* tmp = ptr - prec;
      uint32_t fraction = s*(value - whole);
      ptr = fmtDec(fraction, ptr);
      while (ptr > tmp) *--ptr = '0';
      *--ptr = '.';
    }
    ptr = fmtDec(whole, ptr);
  }
  if (neg) {
    *--ptr = '-';
  }
  return write(ptr, buf + sizeof(buf) - ptr);
}
//------------------------------------------------------------------------------
/** Print a number followed by a field terminator.
 * \param[in] value The number to be printed.
 * \param[in] term The field terminator.  Use '\\n' for CR LF.
 * \return The number of bytes written or -1 if an error occurs.
 */
int SdBaseFile::printField(uint16_t value, char term) {
  return printFieldT(this, value, term, false);
}
//------------------------------------------------------------------------------
/** Print a number followed by a field terminator.
 * \param[in] value The number to be printed.
 * \param[in] term The field terminator.  Use '\\n' for CR LF.
 * \return The number of bytes written or -1 if an error occurs.
 */
int SdBaseFile::printField(int16_t value, char term) {
  bool neg = false;
  if (value < 0) {
    value = -value;
    neg = true;
  }
  return printFieldT(this, (uint16_t)value, term, neg);
}
//------------------------------------------------------------------------------
/** Print a number followed by a field terminator.
 * \param[in] value The number to be printed.
 * \param[in] term The field terminator.  Use '\\n' for CR LF.
 * \return The number of bytes written or -1 if an error occurs.
 */
int SdBaseFile::printField(uint32_t value, char term) {
  return printFieldT(this, value, term, false);
}
//------------------------------------------------------------------------------
/** Print a number followed by a field terminator.
 * \param[in] value The number to be printed.
 * \param[in] term The field terminator.  Use '\\n' for CR LF.
 * \return The number of bytes written or -1 if an error occurs.
 */
int SdBaseFile::printField(int32_t value, char term) {
  bool neg = false;
  if (value < 0) {
    value = -value;
    neg = true;
  }
  return printFieldT(this, (uint32_t)value, term, neg);
}
//------------------------------------------------------------------------------
/** Print a file's modify date and time
 *
 * \param[in] pr Print stream for output.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool SdBaseFile::printModifyDateTime(Print* pr) {
  dir_t dir;
  if (!dirEntry(&dir)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  printFatDate(pr, dir.lastWriteDate);
  pr->write(' ');
  printFatTime(pr, dir.lastWriteTime);
  return true;

 fail:
  return false;
}
//------------------------------------------------------------------------------
/** Print a file's name
 *
 * \param[in] pr Print stream for output.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool SdBaseFile::printName(Print* pr) {
  char name[13];
  if (!getFilename(name)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  return pr->print(name) > 0;

 fail:
  return false;
}
//------------------------------------------------------------------------------
/** Print a file's name to stdOut
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool SdBaseFile::printName() {
  return printName(SdFat::stdOut());
}
