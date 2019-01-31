/*
  This file is part of the ArduinoECCX08 library.
  Copyright (c) 2019 Arduino SA. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "ASN1Utils.h"

int ASN1UtilsClass::versionLength()
{
  return 3;
}

int ASN1UtilsClass::issuerOrSubjectLength(const String& countryName,
                                          const String& stateProvinceName,
                                          const String& localityName,
                                          const String& organizationName,
                                          const String& organizationalUnitName,
                                          const String& commonName)
{
  int length                       = 0;
  int countryNameLength            = countryName.length();
  int stateProvinceNameLength      = stateProvinceName.length();
  int localityNameLength           = localityName.length();
  int organizationNameLength       = organizationName.length();
  int organizationalUnitNameLength = organizationalUnitName.length();
  int commonNameLength             = commonName.length();

  if (countryNameLength) {
    length += (11 + countryNameLength);
  }

  if (stateProvinceNameLength) {
    length += (11 + stateProvinceNameLength);
  }

  if (localityNameLength) {
    length += (11 + localityNameLength);
  }

  if (organizationNameLength) {
    length += (11 + organizationNameLength);
  }

  if (organizationalUnitNameLength) {
    length += (11 + organizationalUnitNameLength);
  }

  if (commonNameLength) {
    length += (11 + commonNameLength);
  }

  return length;
}

int ASN1UtilsClass::publicKeyLength()
{
  return (2 + 2 + 9 + 10 + 4 + 64);
}

int ASN1UtilsClass::signatureLength(const byte signature[])
{
  const byte* r = &signature[0];
  const byte* s = &signature[32];

  int rLength = 32;
  int sLength = 32;

  while (*r == 0x00 && rLength) {
    r++;
    rLength--;
  }

  if (*r & 0x80) {
    rLength++;
  }

  while (*s == 0x00 && sLength) {
    s++;
    sLength--;
  }

  if (*s & 0x80) {
    sLength++;
  }

  return (21 + rLength + sLength);
}

int ASN1UtilsClass::serialNumberLength(const byte serialNumber[], int length)
{
  while (*serialNumber == 0 && length) {
    serialNumber++;
    length--;
  }

  if (length && *serialNumber & 0x80) {
    length++;
  }

  return (2 + length);
}

int ASN1UtilsClass::sequenceHeaderLength(int length)
{
  if (length > 255) {
    return 4;
  } else if (length > 127) {
    return 3;
  } else {
    return 2;
  }
}

void ASN1UtilsClass::appendVersion(int version, byte out[])
{
  out[0] = ASN1_INTEGER;
  out[1] = 0x01;
  out[2] = version;
}

void ASN1UtilsClass::appendIssuerOrSubject(const String& countryName,
                                            const String& stateProvinceName,
                                            const String& localityName,
                                            const String& organizationName,
                                            const String& organizationalUnitName,
                                            const String& commonName,
                                            byte out[])
{
  if (countryName.length() > 0) {
    out += appendName(countryName, 0x06, out);
  }

  if (stateProvinceName.length() > 0) {
    out += appendName(stateProvinceName, 0x08, out);
  }

  if (localityName.length() > 0) {
    out += appendName(localityName, 0x07, out);
  }

  if (organizationName.length() > 0) {
    out += appendName(organizationName, 0x0a, out);
  }

  if (organizationalUnitName.length() > 0) {
    out += appendName(organizationalUnitName, 0x0b, out);
  }

  if (commonName.length() > 0) {
    out += appendName(commonName, 0x03, out);
  }
}

void ASN1UtilsClass::appendPublicKey(const byte publicKey[], byte out[])
{
  int subjectPublicKeyDataLength = 2 + 9 + 10 + 4 + 64;

  // subject public key
  *out++ = ASN1_SEQUENCE;
  *out++ = (subjectPublicKeyDataLength) & 0xff;

  *out++ = ASN1_SEQUENCE;
  *out++ = 0x13;

  // EC public key
  *out++ = ASN1_OBJECT_IDENTIFIER;
  *out++ = 0x07;
  *out++ = 0x2a;
  *out++ = 0x86;
  *out++ = 0x48;
  *out++ = 0xce;
  *out++ = 0x3d;
  *out++ = 0x02;
  *out++ = 0x01;

  // PRIME 256 v1
  *out++ = ASN1_OBJECT_IDENTIFIER;
  *out++ = 0x08;
  *out++ = 0x2a;
  *out++ = 0x86;
  *out++ = 0x48;
  *out++ = 0xce;
  *out++ = 0x3d;
  *out++ = 0x03;
  *out++ = 0x01;
  *out++ = 0x07;

  *out++ = 0x03;
  *out++ = 0x42;
  *out++ = 0x00;
  *out++ = 0x04;

  memcpy(out, publicKey, 64);
}

void ASN1UtilsClass::appendSignature(const byte signature[], byte out[])
{
  // signature algorithm
  *out++ = ASN1_SEQUENCE;
  *out++ = 0x0a;
  *out++ = ASN1_OBJECT_IDENTIFIER;
  *out++ = 0x08;

  // ECDSA with SHA256
  *out++ = 0x2a;
  *out++ = 0x86;
  *out++ = 0x48;
  *out++ = 0xce;
  *out++ = 0x3d;
  *out++ = 0x04;
  *out++ = 0x03;
  *out++ = 0x02;

  const byte* r = &signature[0];
  const byte* s = &signature[32];

  int rLength = 32;
  int sLength = 32;

  while (*r == 0 && rLength) {
    r++;
    rLength--;
  }

  while (*s == 0 && sLength) {
    s++;
    sLength--;
  }

  if (*r & 0x80) {
    rLength++;  
  }

  if (*s & 0x80) {
    sLength++;
  }

  *out++ = ASN1_BIT_STRING;
  *out++ = (rLength + sLength + 7);
  *out++ = 0;

  *out++ = ASN1_SEQUENCE;
  *out++ = (rLength + sLength + 4);

  *out++ = ASN1_INTEGER;
  *out++ = rLength;
  if ((*r & 0x80) && rLength) {
    *out++ = 0;
    rLength--;
  }
  memcpy(out, r, rLength);
  out += rLength;

  *out++ = ASN1_INTEGER;
  *out++ = sLength;
  if ((*s & 0x80) && sLength) {
    *out++ = 0;
    sLength--;
  }
  memcpy(out, s, sLength);
  out += rLength;
}

void ASN1UtilsClass::appendSerialNumber(const byte serialNumber[], int length, byte out[])
{
  while (*serialNumber == 0 && length) {
    serialNumber++;
    length--;
  }

  if (length && *serialNumber & 0x80) {
    length++;  
  }

  *out++ = ASN1_INTEGER;
  *out++ = length;

  if (length && *serialNumber & 0x80) {
    *out++ = 0x00;
    length--;
  }

  memcpy(out, serialNumber, length);
}

int ASN1UtilsClass::appendName(const String& name, int type, byte out[])
{
  int nameLength = name.length();

  *out++ = ASN1_SET;
  *out++ = nameLength + 9;

  *out++ = ASN1_SEQUENCE;
  *out++ = nameLength + 7;

  *out++ = ASN1_OBJECT_IDENTIFIER;
  *out++ = 0x03;
  *out++ = 0x55;
  *out++ = 0x04;
  *out++ = type;

  *out++ = ASN1_PRINTABLE_STRING;
  *out++ = nameLength;
  memcpy(out, name.c_str(), nameLength);

  return (nameLength + 11);
}

void ASN1UtilsClass::appendSequenceHeader(int length, byte out[])
{
  *out++ = ASN1_SEQUENCE;
  if (length > 255) {
    *out++ = 0x82;
    *out++ = (length >> 8) & 0xff;
  } else if (length > 127) {
    *out++ = 0x81;
  }
  *out++ = (length) & 0xff;
}


String ASN1UtilsClass::base64Encode(const byte in[], unsigned int length, const char* prefix, const char* suffix)
{
  static const char* CODES = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

  int b;
  String out;

  int reserveLength = 4 * ((length + 2) / 3) + ((length / 3 * 4) / 76) + strlen(prefix) + strlen(suffix);
  out.reserve(reserveLength);

  if (prefix) {
    out += prefix;
  }
  
  for (unsigned int i = 0; i < length; i += 3) {
    if (i > 0 && (i / 3 * 4) % 76 == 0) { 
      out += '\n'; 
    }

    b = (in[i] & 0xFC) >> 2;
    out += CODES[b];

    b = (in[i] & 0x03) << 4;
    if (i + 1 < length) {
      b |= (in[i + 1] & 0xF0) >> 4;
      out += CODES[b];
      b = (in[i + 1] & 0x0F) << 2;
      if (i + 2 < length) {
         b |= (in[i + 2] & 0xC0) >> 6;
         out += CODES[b];
         b = in[i + 2] & 0x3F;
         out += CODES[b];
      } else {
        out += CODES[b];
        out += '=';
      }
    } else {
      out += CODES[b];
      out += "==";
    }
  }

  if (suffix) {
    out += suffix;
  }

  return out;
}

int ASN1UtilsClass::appendDate(int year, int month, int day, int hour, int minute, int second, byte out[])
{
  bool useGeneralizedTime = (year > 2049);

  if (useGeneralizedTime) {
    *out++ = 0x18;
    *out++ = 0x0f;
    *out++ = '0' + (year / 1000);
    *out++ = '0' + ((year % 1000) / 100);
    *out++ = '0' + ((year % 100) / 10);
    *out++ = '0' + (year % 10);
  } else {
    year -= 2000;

    *out++ = 0x17;
    *out++ = 0x0d;
    *out++ = '0' + (year / 10);
    *out++ = '0' + (year % 10);
  }
  *out++ = '0' + (month / 10);
  *out++ = '0' + (month % 10);
  *out++ = '0' + (day / 10);
  *out++ = '0' + (day % 10);
  *out++ = '0' + (hour / 10);
  *out++ = '0' + (hour % 10);
  *out++ = '0' + (minute / 10);
  *out++ = '0' + (minute % 10);
  *out++ = '0' + (second / 10);
  *out++ = '0' + (second % 10);
  *out++ = 0x5a; // UTC

  return (useGeneralizedTime ? 17 : 15);
}

int ASN1UtilsClass::appendEcdsaWithSHA256(byte out[])
{
  *out++ = ASN1_SEQUENCE;
  *out++ = 0x0A;
  *out++ = ASN1_OBJECT_IDENTIFIER;
  *out++ = 0x08;
  *out++ = 0x2A;
  *out++ = 0x86;
  *out++ = 0x48;
  *out++ = 0xCE;
  *out++ = 0x3D;
  *out++ = 0x04;
  *out++ = 0x03;
  *out++ = 0x02;

  return 12;
}

ASN1UtilsClass ASN1Utils;
