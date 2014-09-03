#include "utf.h"
#include "unicode_data.h"

using namespace std;
using namespace utf;

// exception constructor
utf::encode_error::encode_error(string error_message) {
  message = error_message;
}

// get the error message
string utf::encode_error::get_message() {
  return message;
}

encoding_type utf::detect_encoding(const string &input) {
  // look for 4-byte BOM
  if (input.size() >= 4) {
    // UTF32BE
    if ((uint8_t)input[0] == 0x00 &&
        (uint8_t)input[1] == 0x00 &&
        (uint8_t)input[2] == 0xFE &&
        (uint8_t)input[3] == 0xFF) {
      if (is_valid(input, ENCODING_UTF32BE))
        return ENCODING_UTF32BE;
    }

    // UTF32LE
    if ((uint8_t)input[0] == 0xFF &&
        (uint8_t)input[1] == 0xFE &&
        (uint8_t)input[2] == 0x00 &&
        (uint8_t)input[3] == 0x00) {
      if (is_valid(input, ENCODING_UTF32LE))
        return ENCODING_UTF32LE;
    }
  }

  // look for 2-byte BOM
  if (input.size() >= 2) {
    // UTF16BE
    if ((uint8_t)input[0] == 0xFE &&
        (uint8_t)input[1] == 0xFF) {
      if (is_valid(input, ENCODING_UTF16BE))
        return ENCODING_UTF16BE;
    }

    // UTF16LE
    if ((uint8_t)input[0] == 0xFF &&
        (uint8_t)input[1] == 0xFE) {
      if (is_valid(input, ENCODING_UTF16LE))
        return ENCODING_UTF16LE;
    }
  }

  // ASCII
  if (is_valid(input, ENCODING_ASCII))
    return ENCODING_ASCII;

  // UTF8
  if (is_valid(input, ENCODING_UTF8))
    return ENCODING_UTF8;

  // unknown encoding
  return ENCODING_UNKNOWN;
}

bool utf::is_valid(const string &input, encoding_type encoding) {
  // start at the beginning
  size_t pos = 0;
  while (pos < input.size()) {
    // check if the character is valid
    size_t size = get_char_size(input, pos, encoding);
    if (size == 0)
      return false;
    
    // move to the next character
    pos += size;
  }

  // if we didn't fail yet, return success
  return true;
}

string utf::convert_encoding(const string &input, encoding_type input_encoding, encoding_type output_encoding, bool include_bom) {
  // basic error checking
  if (input_encoding != ENCODING_ASCII && input_encoding != ENCODING_UTF8 &&
      input_encoding != ENCODING_UTF16BE && input_encoding != ENCODING_UTF16LE &&
      input_encoding != ENCODING_UTF32BE && input_encoding != ENCODING_UTF32LE)
    throw encode_error("unknown input encoding");
  if (output_encoding != ENCODING_ASCII && output_encoding != ENCODING_UTF8 &&
      output_encoding != ENCODING_UTF16BE && output_encoding != ENCODING_UTF16LE &&
      output_encoding != ENCODING_UTF32BE && output_encoding != ENCODING_UTF32LE)
    throw encode_error("unknown output encoding");
  if (!is_valid(input, input_encoding))
    throw encode_error("malformed input string");

  // store the result
  string result;

  // add the BOM if necessary
  if (include_bom) {
    // UTF8
    if (output_encoding == ENCODING_UTF8) {
      result.push_back(0xEF);
      result.push_back(0xBB);
      result.push_back(0xBF);
    }

    // UTF16BE
    if (output_encoding == ENCODING_UTF16BE) {
      result.push_back(0xFE);
      result.push_back(0xFF);
    }

    // UTF16LE
    if (output_encoding == ENCODING_UTF16LE) {
      result.push_back(0xFF);
      result.push_back(0xFE);
    }

    // UTF32BE
    if (output_encoding == ENCODING_UTF32BE) {
      result.push_back(0x00);
      result.push_back(0x00);
      result.push_back(0xFE);
      result.push_back(0xFF);
    }

    // UTF32LE
    if (output_encoding == ENCODING_UTF32LE) {
      result.push_back(0xFF);
      result.push_back(0xFE);
      result.push_back(0x00);
      result.push_back(0x00);
    }
  }

  // start at the beginning
  size_t pos = 0;

  // skip the BOM in the input if present
  if (input_encoding == ENCODING_UTF8) {
    if (input.size() >= 3) {
      if ((uint8_t)input[0] == 0xEF &&
          (uint8_t)input[1] == 0xBB &&
          (uint8_t)input[2] == 0xBF)
        pos += 3;
    }
  }
  if (input_encoding == ENCODING_UTF16BE) {
    if (input.size() >= 2) {
      if ((uint8_t)input[0] == 0xFE &&
          (uint8_t)input[1] == 0xFF)
        pos += 2;
    }
  }
  if (input_encoding == ENCODING_UTF16LE) {
    if (input.size() >= 2) {
      if ((uint8_t)input[0] == 0xFF &&
          (uint8_t)input[1] == 0xFE)
        pos += 2;
    }
  }
  if (input_encoding == ENCODING_UTF32BE) {
    if (input.size() >= 4) {
      if ((uint8_t)input[0] == 0x00 &&
          (uint8_t)input[1] == 0x00 &&
          (uint8_t)input[2] == 0xFE &&
          (uint8_t)input[3] == 0xFF)
        pos += 4;
    }
  }
  if (input_encoding == ENCODING_UTF32LE) {
    if (input.size() >= 4) {
      if ((uint8_t)input[0] == 0xFF &&
          (uint8_t)input[1] == 0xFE &&
          (uint8_t)input[2] == 0x00 &&
          (uint8_t)input[3] == 0x00)
        pos += 4;
    }
  }

  // iterate through the input string
  while (pos < input.size()) {
    // add the code point to the output string
    add_char(result, get_char(input, pos, input_encoding), output_encoding);

    // advance to the next code point
    pos += get_char_size(input, pos, input_encoding);
  }

  // return the result
  return result;
}

size_t utf::get_length(const std::string &input, encoding_type encoding) {
  // do a linear walk through the string to count each code point
  size_t size = 0;
  size_t pos = 0;
  while (pos < input.size()) {
    size_t char_size = get_char_size(input, pos, encoding);
    if (!char_size)
      throw encode_error("invalid code point");
    pos += char_size;
    ++size;
  }
  return size;
}

size_t utf::get_char_size(const string &input, size_t pos, encoding_type encoding) {
  // check the range of pos
  if (pos >= input.size())
    throw encode_error("index out of range");

  // ASCII
  if (encoding == ENCODING_ASCII) {
    // make sure the code point is within the valid range
    if ((unsigned char)input[pos] <= 127)
      return 1;
    return 0;
  }

  // UTF8
  if (encoding == ENCODING_UTF8) {
    // one byte
    if ((uint8_t)input[pos] < 0x80)
      return 1;

    // two bytes
    if ((uint8_t)input[pos] >= 0xC0 && (uint8_t)input[pos] < 0xE0) {
      if (pos+1 < input.size()) {
        if ((uint8_t)input[pos+1] >= 0x80 && (uint8_t)input[pos+1] < 0xC0)
          return 2;
      }
    }

    // three bytes
    if ((uint8_t)input[pos] >= 0xE0 && (uint8_t)input[pos] < 0xF0) {
      if (pos+2 < input.size()) {
        if ((uint8_t)input[pos+1] >= 0x80 && (uint8_t)input[pos+1] < 0xC0 &&
            (uint8_t)input[pos+2] >= 0x80 && (uint8_t)input[pos+2] < 0xC0)
          return 3;
      }
    }

    // four bytes
    if ((uint8_t)input[pos] >= 0xF0 && (uint8_t)input[pos] < 0xF8) {
      if (pos+3 < input.size()) {
        if ((uint8_t)input[pos+1] >= 0x80 && (uint8_t)input[pos+1] < 0xC0 &&
            (uint8_t)input[pos+2] >= 0x80 && (uint8_t)input[pos+2] < 0xC0 &&
            (uint8_t)input[pos+3] >= 0x80 && (uint8_t)input[pos+3] < 0xC0) {
          // calculate the code point
          uint32_t code_point = ((((uint8_t)input[pos])&0x07)<<18)+((((uint8_t)input[pos+1])&0x3F)<<12)+((((uint8_t)input[pos+2])&0x3F)<<6)+(((uint8_t)input[pos+3])&0x3F);

          // make sure the code point is within the valid range
          if (code_point <= 0x10FFFF)
            return 4;
        }
      }
    }

    // invalid
    return 0;
  }

  // UTF16BE
  if (encoding == ENCODING_UTF16BE) {
    // make sure there are 2 bytes
    if (pos+1 >= input.size())
      return 0;

    // get the high surrogate
    uint16_t high = (((uint8_t)input[pos])<<8)+((uint8_t)input[pos+1]);

    // two bytes
    if (high < 0xD800 || high > 0xDFFF)
      return 2;

    // make sure the high surrogate is within the valid range
    if (high > 0xDBFF)
      return 0;

    // make sure there are 4 bytes
    if (pos+3 >= input.size())
      return 0;

    // get the low surrogate
    uint16_t low = (((uint8_t)input[pos+2])<<8)+((uint8_t)input[pos+3]);

    // make sure the low surrogate is within the valid range
    if (low < 0xDC00 || low > 0xDFFF)
      return 0;

    // calculate the code point
    uint32_t code_point = 0x10000+((high-0xD800)<<10)+(low-0xDC00);

    // make sure the code point is within the valid range
    if (code_point <= 0x10FFFF)
      return 4;

    // invalid
    return 0;
  }

  // UTF16LE
  if (encoding == ENCODING_UTF16LE) {
    // make sure there are 2 bytes
    if (pos+1 >= input.size())
      return 0;

    // get the high surrogate
    uint16_t high = (((uint8_t)input[pos+1])<<8)+((uint8_t)input[pos]);

    // two bytes
    if (high < 0xD800 || high > 0xDFFF)
      return 2;

    // make sure the high surrogate is within the valid range
    if (high > 0xDBFF)
      return 0;

    // make sure there are 4 bytes
    if (pos+3 >= input.size())
      return 0;

    // get the low surrogate
    uint16_t low = (((uint8_t)input[pos+3])<<8)+((uint8_t)input[pos+2]);

    // make sure the low surrogate is within the valid range
    if (low < 0xDC00 || low > 0xDFFF)
      return 0;

    // calculate the code point
    uint32_t code_point = 0x10000+((high-0xD800)<<10)+(low-0xDC00);

    // make sure the code point is within the valid range
    if (code_point <= 0x10FFFF)
      return 4;

    // invalid
    return 0;
  }

  // UTF32BE
  if (encoding == ENCODING_UTF32BE) {
    // make sure there are 4 bytes
    if (pos+3 >= input.size())
      return 0;

    // calculate the code point
    uint32_t code_point = (((uint8_t)input[pos])<<24)+(((uint8_t)input[pos+1])<<16)+(((uint8_t)input[pos+2])<<8)+((uint8_t)input[pos+3]);

    // make sure the code point is within the valid range
    if (code_point <= 0x10FFFF)
      return 4;

    // invalid
    return 0;
  }

  // UTF32LE
  if (encoding == ENCODING_UTF32LE) {
    // make sure there are 4 bytes
    if (pos+3 >= input.size())
      return 0;

    // calculate the code point
    uint32_t code_point = (((uint8_t)input[pos+3])<<24)+(((uint8_t)input[pos+2])<<16)+(((uint8_t)input[pos+1])<<8)+((uint8_t)input[pos]);

    // make sure the code point is within the valid range
    if (code_point <= 0x10FFFF)
      return 4;

    // invalid
    return 0;
  }

  // unknown input encoding
  throw encode_error("unknown input encoding");
  return 0;
}

uint32_t utf::get_char(const string &input, size_t pos, encoding_type encoding) {
  // make sure there is a character at pos
  size_t size = get_char_size(input, pos, encoding);
  if (size == 0)
    throw encode_error("index does not refer to a valid code point");
  // ASCII
  if (encoding == ENCODING_ASCII)
    return (unsigned char)input[pos];

  // UTF8
  if (encoding == ENCODING_UTF8) {
    if (size == 1)
      return input[pos];
    if (size == 2)
      return ((((uint8_t)input[pos])&0x1F)<<6)+(((uint8_t)input[pos+1])&0x3F);
    if (size == 3)
      return ((((uint8_t)input[pos])&0x0F)<<12)+((((uint8_t)input[pos+1])&0x3F)<<6)+(((uint8_t)input[pos+2])&0x3F);
    if (size == 4)
      return ((((uint8_t)input[pos])&0x07)<<18)+((((uint8_t)input[pos+1])&0x3F)<<12)+((((uint8_t)input[pos+2])&0x3F)<<6)+(((uint8_t)input[pos+3])&0x3F);
  }

  // UTF16BE
  if (encoding == ENCODING_UTF16BE) {
    if (size == 2)
      return (((uint8_t)input[pos])<<8)+((uint8_t)input[pos+1]);
    if (size == 4)
      return 0x10000+((((((uint8_t)input[pos])<<8)+((uint8_t)input[pos+1]))-0xD800)<<10)+(((((uint8_t)input[pos+2])<<8)+((uint8_t)input[pos+3]))-0xDC00);
  }

  // UTF16LE
  if (encoding == ENCODING_UTF16LE) {
    if (size == 2)
      return (((uint8_t)input[pos+1])<<8)+((uint8_t)input[pos]);
    if (size == 4)
      return 0x10000+((((((uint8_t)input[pos+1])<<8)+((uint8_t)input[pos]))-0xD800)<<10)+(((((uint8_t)input[pos+3])<<8)+((uint8_t)input[pos+2]))-0xDC00);
  }

  // UTF32BE
  if (encoding == ENCODING_UTF32BE)
    return (((uint8_t)input[pos])<<24)+(((uint8_t)input[pos+1])<<16)+(((uint8_t)input[pos+2])<<8)+((uint8_t)input[pos+3]);

  // UTF32LE
  if (encoding == ENCODING_UTF32LE)
    return (((uint8_t)input[pos+3])<<24)+(((uint8_t)input[pos+2])<<16)+(((uint8_t)input[pos+1])<<8)+((uint8_t)input[pos]);

  // unknown input encoding
  throw encode_error("unknown input encoding");
  return 0;
}

void utf::set_char(string &input, size_t pos, uint32_t code_point, encoding_type encoding) {
  // get the size of the code point to replace
  size_t old_size = get_char_size(input, pos, encoding);

  // make sure the code point is valid
  if (old_size == 0)
    throw encode_error("index does not refer to a valid code point");

  // get the size of the new code point
  string new_code_point;
  add_char(new_code_point, code_point, encoding);
  if (old_size == new_code_point.size()) {
    for (size_t i = 0; i < old_size; i++)
      input[pos+i] = new_code_point[i];
  } else
    input = input.substr(0, pos)+new_code_point+input.substr(pos+new_code_point.size(), input.size()-(pos+new_code_point.size()));
}

void utf::add_char(string &input, uint32_t code_point, encoding_type encoding) {
  // make sure the code point is within the valid range
  if (code_point > 0x10FFFF)
    throw encode_error("invalid code point");

  // ASCII
  if (encoding == ENCODING_ASCII) {
    // make sure the code point is within the valid range
    if ((unsigned char)code_point > 127)
      throw encode_error("invalid code point for ASCII");

    // add the code point
    input.push_back(code_point);
    return;
  }

  // UTF8
  if (encoding == ENCODING_UTF8) {
    // one byte
    if (code_point <= 0x0000007F) {
      input.push_back(code_point);
      return;
    }

    // two bytes
    if (code_point >= 0x00000080 && code_point <= 0x000007FF) {
      input.push_back(0xC0+(code_point>>6));
      input.push_back(0x80+(code_point&0x3F));
      return;
    }

    // three bytes
    if (code_point >= 0x00000800 && code_point <= 0x0000FFFF) {
      input.push_back(0xE0+(code_point>>12));
      input.push_back(0x80+((code_point>>6)&0x3F));
      input.push_back(0x80+(code_point&0x3F));
      return;
    }

    // four bytes
    if (code_point >= 0x00010000 && code_point <= 0x0010FFFF) {
      input.push_back(0xF0+(code_point>>18));
      input.push_back(0x80+((code_point>>12)&0x3F));
      input.push_back(0x80+((code_point>>6)&0x3F));
      input.push_back(0x80+(code_point&0x3F));
      return;
    }
  }

  // UTF16BE
  if (encoding == ENCODING_UTF16BE) {
    if (code_point >= 0xD800 && code_point <= 0xDFFF)
      throw encode_error("unable to encode code points U+D800 to U+DFFF in UTF-16");
    if (code_point <= 0xD7FF || (code_point >= 0xE000 && code_point <= 0xFFFF)) {
      input.push_back(code_point>>8);
      input.push_back(code_point&0xFF);
      return;
    }
    code_point -= 0x10000;
    uint16_t high = (code_point>>10)+0xD800;
    uint16_t low = (code_point&0x3FF)+0xDC00;
    input.push_back(high>>8);
    input.push_back(high&0xFF);
    input.push_back(low>>8);
    input.push_back(low&0xFF);
    return;
  }

  // UTF16LE
  if (encoding == ENCODING_UTF16LE) {
    if (code_point >= 0xD800 && code_point <= 0xDFFF)
      throw encode_error("unable to encode code points U+D800 to U+DFFF in UTF-16");
    if (code_point <= 0xD7FF || (code_point >= 0xE000 && code_point <= 0xFFFF)) {
      input.push_back(code_point&0xFF);
      input.push_back(code_point>>8);
      return;
    }
    code_point -= 0x10000;
    uint16_t high = (code_point>>10)+0xD800;
    uint16_t low = (code_point&0x3FF)+0xDC00;
    input.push_back(high&0xFF);
    input.push_back(high>>8);
    input.push_back(low&0xFF);
    input.push_back(low>>8);
    return;
  }

  // UTF32BE
  if (encoding == ENCODING_UTF32BE) {
    input.push_back(code_point>>24);
    input.push_back((code_point>>16)&0xFF);
    input.push_back((code_point>>8)&0xFF);
    input.push_back(code_point&0xFF);
    return;
  }

  // UTF32LE
  if (encoding == ENCODING_UTF32LE) {
    input.push_back(code_point&0xFF);
    input.push_back((code_point>>8)&0xFF);
    input.push_back((code_point>>16)&0xFF);
    input.push_back(code_point>>24);
    return;
  }
}

bool utf::is_alpha(uint32_t code_point) {
  // make sure the code point is within the valid range
  if (code_point > 0x10FFFF)
    throw encode_error("invalid code point");

  // search the table
  for (size_t i = 0; i < NUM_LETTERS; i++) {
    if (code_point == letters[i].code_point)
      return true;
  }
  return false;
}

bool utf::is_upper(uint32_t code_point) {
  // make sure the code point is within the valid range
  if (code_point > 0x10FFFF)
    throw encode_error("invalid code point");

  // search the table
  for (size_t i = 0; i < NUM_LETTERS; i++) {
    if (code_point == letters[i].code_point && letters[i].character_case == CASE_UPPER)
      return true;
  }
  return false;
}

bool utf::is_lower(uint32_t code_point) {
  // make sure the code point is within the valid range
  if (code_point > 0x10FFFF)
    throw encode_error("invalid code point");

  // search the table
  for (size_t i = 0; i < NUM_LETTERS; i++) {
    if (code_point == letters[i].code_point && letters[i].character_case == CASE_LOWER)
      return true;
  }
  return false;
}

bool utf::is_title(uint32_t code_point) {
  // make sure the code point is within the valid range
  if (code_point > 0x10FFFF)
    throw encode_error("invalid code point");

  // search the table
  for (size_t i = 0; i < NUM_LETTERS; i++) {
    if (code_point == letters[i].code_point && letters[i].character_case == CASE_TITLE)
      return true;
  }
  return false;
}

bool utf::is_numeric(uint32_t code_point) {
  // make sure the code point is within the valid range
  if (code_point > 0x10FFFF)
    throw encode_error("invalid code point");

  // search the table
  for (size_t i = 0; i < NUM_NUMBERS; i++) {
    if (code_point == numbers[i])
      return true;
  }
  return false;
}

bool utf::is_whitespace(uint32_t code_point) {
  // make sure the code point is within the valid range
  if (code_point > 0x10FFFF)
    throw encode_error("invalid code point");

  // search the table
  for (size_t i = 0; i < NUM_WHITESPACE; i++) {
    if (code_point == whitespace[i])
      return true;
  }
  return false;
}

bool utf::is_newline(uint32_t code_point) {
  // make sure the code point is within the valid range
  if (code_point > 0x10FFFF)
    throw encode_error("invalid code point");

  // search the table
  for (size_t i = 0; i < NUM_NEWLINES; i++) {
    if (code_point == newlines[i])
      return true;
  }
  return false;
}

uint32_t utf::to_upper(uint32_t code_point) {
  // make sure the code point is within the valid range
  if (code_point > 0x10FFFF)
    throw encode_error("invalid code point");

  // search the table
  for (size_t i = 0; i < NUM_LETTERS; i++) {
    if (code_point == letters[i].code_point) {
      if (letters[i].upper_version != CODE_POINT_NULL)
        return letters[i].upper_version;
      break;
    }
  }
  return code_point;
}

uint32_t utf::to_lower(uint32_t code_point) {
  // make sure the code point is within the valid range
  if (code_point > 0x10FFFF)
    throw encode_error("invalid code point");

  // search the table
  for (size_t i = 0; i < NUM_LETTERS; i++) {
    if (code_point == letters[i].code_point) {
      if (letters[i].lower_version != CODE_POINT_NULL)
        return letters[i].lower_version;
      break;
    }
  }
  return code_point;
}

uint32_t utf::to_title(uint32_t code_point) {
  // make sure the code point is within the valid range
  if (code_point > 0x10FFFF)
    throw encode_error("invalid code point");

  // search the table
  for (size_t i = 0; i < NUM_LETTERS; i++) {
    if (code_point == letters[i].code_point) {
      if (letters[i].title_version != CODE_POINT_NULL)
        return letters[i].title_version;
      break;
    }
  }
  return code_point;
}