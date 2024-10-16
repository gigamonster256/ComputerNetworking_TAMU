#ifndef __NETASCII_H__
#define __NETASCII_H__

#include <fstream>
#include <iostream>
#include <streambuf>
#include <string>

class UNIXtoNetasciiStreamBuf : public std::streambuf {
  static const std::size_t bufferSize = 2;
  std::istream& file;
  char buffer[bufferSize];

 public:
  UNIXtoNetasciiStreamBuf(std::istream& f) : file(f) {
    setg(buffer, buffer, buffer);
  }

 protected:
  int underflow() override {
    if (gptr() < egptr()) {
      return traits_type::to_int_type(*gptr());
    }

    file.read(buffer, 1);
    std::streamsize bytesRead = file.gcount();

    if (bytesRead == 0) {
      return EOF;
    }

    // LF -> CRLF
    // CRLF -> CRLF
    // CR!LF -> CR\0
    if (buffer[0] == '\n') {
      buffer[0] = '\r';
      buffer[1] = '\n';
      bytesRead = 2;
    } else if (buffer[0] == '\r') {
      // Peek at the next character
      char nextChar = file.peek();
      if (nextChar == '\n') {
        file.read(buffer + 1, 1);
      } else {
        buffer[1] = '\0';
      }
      bytesRead = 2;
    }

    setg(buffer, buffer, buffer + bytesRead);

    return traits_type::to_int_type(*gptr());
  }
};

class UNIXtoNetasciiStream : public std::istream {
  UNIXtoNetasciiStreamBuf buf;

 public:
  UNIXtoNetasciiStream(std::istream& f) : std::istream(&buf), buf(f) {}
};

class NetasciitoUNIXStreamBuf : public std::streambuf {
  std::istream& file;
  char buffer;

 public:
  NetasciitoUNIXStreamBuf(std::istream& f) : file(f) {
    setg(&buffer, &buffer, &buffer);
  }

 protected:
  int underflow() override {
    if (gptr() < egptr()) {
      return traits_type::to_int_type(*gptr());
    }

    file.read(&buffer, 1);
    std::streamsize bytesRead = file.gcount();

    if (bytesRead == 0) {
      return EOF;
    }

    // CRLF -> LF
    // CR\0 -> CR
    if (buffer == '\r') {
      // Peek at the next character
      char nextChar = file.peek();
      if (nextChar == '\n') {
        file.read(&buffer, 1);
      }
      if (nextChar == '\0') {
        file.get();
      }
    }

    setg(&buffer, &buffer, &buffer + bytesRead);

    return traits_type::to_int_type(*gptr());
  }
};

class NetasciitoUNIXStream : public std::istream {
  NetasciitoUNIXStreamBuf buf;

 public:
  NetasciitoUNIXStream(std::istream& f) : std::istream(&buf), buf(f) {}
};

#endif  // __NETASCII_H__
