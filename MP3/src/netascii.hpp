#pragma once

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
    // CR -> CR\0 (always, even when immediately followed by LF)
    if (buffer[0] == '\n') {
      buffer[0] = '\r';
      buffer[1] = '\n';
      bytesRead = 2;
    } else if (buffer[0] == '\r') {
      // A CR in the data is always escaped as CR NUL (RFC 764), even when it
      // is immediately followed by an LF; the LF is then encoded as its own
      // CRLF on the next iteration. Collapsing "CRLF" into a single line
      // ending is lossy and makes the transform non-invertible.
      buffer[1] = '\0';
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
