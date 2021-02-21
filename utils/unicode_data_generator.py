# -*- coding: utf-8 -*-

import csv
import urllib.request

UNICODE_DATA_FILE_URL = 'https://www.unicode.org/Public/13.0.0/ucd/UnicodeData.txt'
DATA_FIELD_NAMES = ('code', 'name', 'category', 'unused1', 'unused2', 'unused3', 'unused4',
                    'unused5', 'unused6', 'unused7', 'unused8', 'unused9', 'upper', 'lower', 'unused10')

DERIVED_GENERAL_CATEGORY_FILE_URL = 'https://www.unicode.org/Public/13.0.0/ucd/extracted/DerivedGeneralCategory.txt'
DERIVED_GENERAL_CATEGORY_FIELD_NAMES = ('range', 'category')

DERIVED_NUMERIC_TYPE_FILE_URL = 'https://www.unicode.org/Public/13.0.0/ucd/extracted/DerivedNumericType.txt'
DERIVED_NUMERIC_TYPE_FIELD_NAMES = ('range', 'type')


def create_reader(url, fieldnames):
    stream = urllib.request.urlopen(url)
    stream = (line.decode().split('#')[0] for line in stream)
    return csv.DictReader(stream, fieldnames=fieldnames, delimiter=';')


def parse_range(range_str):
    range_str = range_str.strip()
    if range_str.find('..') != -1:
        beg, end = range_str.split('..')
    else:
        beg = range_str
        end = beg
    return int(beg, 16), int(end, 16) + 1


def main():
    unicode_data_reader = create_reader(
        UNICODE_DATA_FILE_URL, DATA_FIELD_NAMES)
    uppers = {}
    lowers = {}
    for record in unicode_data_reader:
        code = int('0x' + record['code'].strip(), 16)
        upper = record['upper'].strip()
        if upper:
            upper = int('0x' + upper, 16)
            uppers[code] = upper

        lower = record['lower'].strip()
        if lower:
            lower = int('0x' + lower, 16)
            lowers[code] = lower

    general_category_reader = create_reader(
        DERIVED_GENERAL_CATEGORY_FILE_URL, DERIVED_GENERAL_CATEGORY_FIELD_NAMES)
    categories = {}
    for record in general_category_reader:
        beg, end = parse_range(record['range'])

        for code in range(beg, end):
            categories[code] = 'GeneralCategory::' + record['category'].strip()

    range_category = categories[0] if 0 in categories else 'GeneralCategory::Cn'
    category_ranges = []
    for code in range(0x110000):
        category = categories[code] if code in categories else 'GeneralCategory::Cn'
        if category != range_category:
            category_ranges.append((code, range_category))
            range_category = category
    category_ranges.append((0x110000, range_category))

    numeric_type_reader = create_reader(
        DERIVED_NUMERIC_TYPE_FILE_URL, DERIVED_NUMERIC_TYPE_FIELD_NAMES)
    numeric_types = {}
    for record in numeric_type_reader:
        beg, end = parse_range(record['range'])

        for code in range(beg, end):
            numeric_types[code] = 'NumericType::' + record['type'].strip()

    print('#include "unicode_data.h"')
    print()
    print('namespace unicpp {')
    print()
    print('const std::unordered_map<char32_t, char32_t> kUpperMap = {')
    for (code, upper) in sorted(uppers.items()):
        print(f'    {{ {hex(code)}, {hex(upper)} }},')
    print('};')
    print()
    print('const std::unordered_map<char32_t, char32_t> kLowerMap = {')
    for (code, lower) in sorted(lowers.items()):
        print(f'    {{ {hex(code)}, {hex(lower)} }},')
    print('};')
    print()
    print(
        'const std::map<char32_t, GeneralCategory> kGeneralCategoryRangeMap = {')
    for (range_beg, range_category) in sorted(category_ranges):
        print(f'    {{ {hex(range_beg)}, {range_category} }},')
    print('};')
    print()
    print(
        'const std::unordered_map<char32_t, NumericType> kNumericTypeMap = {')
    for (code, numeric_type) in sorted(numeric_types.items()):
        print(f'    {{ {hex(code)}, {numeric_type} }},')
    print('};')
    print()
    print('}  // namespace unicpp')


if __name__ == '__main__':
    main()
