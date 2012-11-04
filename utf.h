/*
  Portable ASCII and Unicode string manipulation functions for C++.
*/

#ifndef UTF_H
#define UTF_H

#include <string>
#include <stdint.h>

namespace utf {

  // exception for encoding errors
  class encode_error {
    public:
      // constructor
      encode_error(std::string error_message);

      // get the error message
      std::string get_message();

    private:
      // the error message
      std::string message;
  };

  // encoding types
  enum encoding_type {
    ENCODING_UNKNOWN,
    ENCODING_ASCII,
    ENCODING_UTF8,
    ENCODING_UTF16BE,
    ENCODING_UTF16LE,
    ENCODING_UTF32BE,
    ENCODING_UTF32LE,
  };

  // detect the encoding for a string
  encoding_type detect_encoding(const std::string &input);

  // determine whether a string is valid in a particular encoding
  bool is_valid(const std::string &input, encoding_type encoding);

  // convert a string from one encoding to another
  std::string convert_encoding(const std::string &input, encoding_type input_encoding, encoding_type output_encoding, bool include_bom);

  // get the number of code points in a string
  size_t get_length(const std::string &input, encoding_type encoding);

  // return the number of bytes of the code point at pos, or 0 if the byte index does not refer to a valid code point
  size_t get_char_size(const std::string &input, size_t pos, encoding_type encoding);

  // get the code point at a particular byte index
  uint32_t get_char(const std::string &input, size_t pos, encoding_type encoding);

  // set the code point at a particular byte index
  void set_char(std::string &input, size_t pos, uint32_t code_point, encoding_type encoding);

  // add a code point to the end of a string
  void add_char(std::string &input, uint32_t code_point, encoding_type encoding);

  // determine whether a code point is a letter
  bool is_alpha(uint32_t code_point);

  // determine whether a code point is uppercase
  bool is_upper(uint32_t code_point);

  // determine whether a code point is lowercase
  bool is_lower(uint32_t code_point);

  // determine whether a code point is titlecase
  bool is_title(uint32_t code_point);

  // determine whether a code point is a number
  bool is_numeric(uint32_t code_point);

  // determine whether a code point is whitespace
  bool is_whitespace(uint32_t code_point);

  // determine whether a code point is a line separator
  bool is_newline(uint32_t code_point);

  // convert a code point to uppercase (return the input if no uppercase form exists)
  uint32_t to_upper(uint32_t code_point);

  // convert a code point to lowercase (return the input if no lowercase form exists)
  uint32_t to_lower(uint32_t code_point);

  // convert a code point to titlecase (return the input if no titlecase form exists)
  uint32_t to_title(uint32_t code_point);

}

#endif