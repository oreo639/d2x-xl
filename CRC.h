/*
   Datei: CRCCCITT.H
          CRC-CCITT
          Public Domain Implementierung von David M. Howard.
*/

#ifndef __crc_h
#define __crc_h

#include "define.h"

uint Crc16f (tCrc crc, byte* bufP, long bufSize);
uint Crc16 (tCrc crc, byte* bufP, long bufSize);
uint Crc32 (tCrc crc, byte* bufP, long bufSize);

#endif //__crc_h

