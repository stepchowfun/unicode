#!/usr/bin/python -O

# This script generates unicode_data.h, the header that contains information
# such as which code points are considered whitespace, letters, numerals,
# uppercase, etc.

# It is unnecessary to run this script unless a new version of UnicodeData.txt
# is published by the Unicode Consortium.  At the time of this writing, the
# current version is 7.0.

# Upgrade procedure:
# 1. Replace the UnicodeData.txt file in this directory with an updated version
#    from the Unicode Consortium.
# 2. Run this script with no arguments from this directory.
#    It will overwrite unicode_data.h in the parent directory.

CODE_POINT_NULL = "FFFFFFFF"

CASE_NONE = "00"
CASE_UPPER = "01"
CASE_LOWER = "02"
CASE_TITLE = "03"
letters = [] # (code point, case, uppercase version, lowercase version, titlecase version)

numbers = [] # code point

whitespace = ["0009", "000A", "000B", "000C", "000D", "0085"] # code point

newlines = ["000A", "000B", "000C", "000D", "0085"] # code point

# get the lines of the file
for line in open("UnicodeData.txt"):
  # make sure the line isn't empty
  if line.strip() == "":
    continue

  # split the line by semicolons
  parts = [part.strip() for part in line[:-1].strip().split(";")]

  # make sure the line has the correct number of parts
  if len(parts) != 15:
    print "error parsing line:  "+line[:-1].strip()

  # letters
  if parts[12] == "":
    parts[12] = CODE_POINT_NULL
  if parts[13] == "":
    parts[13] = CODE_POINT_NULL
  if parts[14] == "":
    parts[14] = CODE_POINT_NULL
  if parts[2] == "Lu":
    letters.append((parts[0], CASE_UPPER, parts[12], parts[13], parts[14]))
  if parts[2] == "Ll":
    letters.append((parts[0], CASE_LOWER, parts[12], parts[13], parts[14]))
  if parts[2] == "Lt":
    letters.append((parts[0], CASE_TITLE, parts[12], parts[13], parts[14]))
  if parts[2] == "Lm":
    letters.append((parts[0], CASE_NONE, parts[12], parts[13], parts[14]))
  if parts[2] == "Lo":
    letters.append((parts[0], CASE_NONE, parts[12], parts[13], parts[14]))

  # numbers
  if parts[2] == "Nd":
    numbers.append(parts[0])
  if parts[2] == "Nl":
    numbers.append(parts[0])
  if parts[2] == "No":
    numbers.append(parts[0])

  # whitespace
  if parts[2] == "Zs" or parts[2] == "Zl" or parts[2] == "Zp":
    whitespace.append(parts[0])

  # line breaks
  if parts[2] == "Zl" or parts[2] == "Zp":
    newlines.append(parts[0])

# write the header file
f = open("../unicode_data.h", "w")
f.write("/*\n")
f.write("  This header was compiled from unicode_data/unicode_data_parser.py.\n")
f.write("*/\n")
f.write("\n")
f.write("#ifndef UNICODE_DATA_H\n")
f.write("#define UNICODE_DATA_H\n")
f.write("\n")
f.write("#include <stdint.h>\n")
f.write("\n")
f.write("#define CODE_POINT_NULL 0xFFFFFFFF\n")
f.write("\n")
f.write("#define CASE_NONE 0\n")
f.write("#define CASE_UPPER 1\n")
f.write("#define CASE_LOWER 2\n")
f.write("#define CASE_TITLE 3\n")
f.write("\n")
f.write("struct letter {\n")
f.write("  uint32_t code_point;\n")
f.write("  uint8_t character_case;\n")
f.write("  uint32_t upper_version;\n")
f.write("  uint32_t lower_version;\n")
f.write("  uint32_t title_version;\n")
f.write("};\n")
f.write("\n")
f.write("#define NUM_LETTERS "+str(len(letters))+"\n");
f.write("\n")
f.write("static letter letters[] = {\n")
for letter in letters:
  f.write("  {0x"+letter[0]+", 0x"+letter[1]+", 0x"+letter[2]+", 0x"+letter[3]+", 0x"+letter[4]+"},\n")
f.write("};\n")
f.write("\n")
f.write("#define NUM_NUMBERS "+str(len(numbers))+"\n")
f.write("\n")
f.write("static uint32_t numbers[] = {\n")
for code_point in numbers:
  f.write("  0x"+code_point+",\n")
f.write("};\n")
f.write("\n")
f.write("#define NUM_WHITESPACE "+str(len(whitespace))+"\n")
f.write("\n")
f.write("static uint32_t whitespace[] = {\n")
for code_point in whitespace:
  f.write("  0x"+code_point+",\n")
f.write("};\n")
f.write("\n")
f.write("#define NUM_NEWLINES "+str(len(newlines))+"\n")
f.write("\n")
f.write("static uint32_t newlines[] = {\n")
for code_point in newlines:
  f.write("  0x"+code_point+",\n")
f.write("};\n")
f.write("\n")
f.write("#endif\n")
f.close()
