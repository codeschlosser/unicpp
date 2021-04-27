# -*- coding: utf-8 -*-

import csv
import urllib.request

UNICODE_DATA_FILE_URL = 'https://www.unicode.org/Public/13.0.0/ucd/UnicodeData.txt'
DATA_FIELD_NAMES = ('code', 'name', 'category', 'unused1', 'unused2', 'decomposition_mapping', 'unused3',
                    'unused4', 'unused5', 'unused6', 'unused7', 'unused8', 'upper', 'lower', 'unused9')

DERIVED_GENERAL_CATEGORY_FILE_URL = 'https://www.unicode.org/Public/13.0.0/ucd/extracted/DerivedGeneralCategory.txt'
DERIVED_GENERAL_CATEGORY_FIELD_NAMES = ('range', 'category')

DERIVED_NUMERIC_TYPE_FILE_URL = 'https://www.unicode.org/Public/13.0.0/ucd/extracted/DerivedNumericType.txt'
DERIVED_NUMERIC_TYPE_FIELD_NAMES = ('range', 'type')

DERIVED_BIDI_CLASS_FILE_URL = 'https://www.unicode.org/Public/13.0.0/ucd/extracted/DerivedBidiClass.txt'
DERIVED_BIDI_CLASS_FIELD_NAMES = ('range', 'bidiclass')


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


def generate_case_maps():
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


def generate_general_category_map():
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

    print(
        'const std::map<char32_t, GeneralCategory> kGeneralCategoryRangeMap = {')
    for (range_beg, range_category) in sorted(category_ranges):
        print(f'    {{ {hex(range_beg)}, {range_category} }},')
    print('};')
    print()


def generate_numeric_type_map():
    numeric_type_reader = create_reader(
        DERIVED_NUMERIC_TYPE_FILE_URL, DERIVED_NUMERIC_TYPE_FIELD_NAMES)
    numeric_types = {}
    for record in numeric_type_reader:
        beg, end = parse_range(record['range'])

        for code in range(beg, end):
            numeric_types[code] = 'NumericType::' + record['type'].strip()

    print(
        'const std::unordered_map<char32_t, NumericType> kNumericTypeMap = {')
    for (code, numeric_type) in sorted(numeric_types.items()):
        print(f'    {{ {hex(code)}, {numeric_type} }},')
    print('};')
    print()


def generate_bidi_class_map():
    bidi_class_reader = create_reader(
        DERIVED_BIDI_CLASS_FILE_URL, DERIVED_BIDI_CLASS_FIELD_NAMES)
    bidiclasses = {}
    for record in bidi_class_reader:
        beg, end = parse_range(record['range'])

        for code in range(beg, end):
            bidiclasses[code] = 'BidiClass::' + record['bidiclass'].strip()

    range_class = bidiclasses[0] if 0 in bidiclasses else 'BidiClass::None'
    class_ranges = []
    for code in range(0x110000):
        bidiclass = bidiclasses[code] if code in bidiclasses else 'BidiClass::None'
        if bidiclass != range_class:
            class_ranges.append((code, range_class))
            range_class = bidiclass
    class_ranges.append((0x110000, range_class))

    print(
        'const std::map<char32_t, BidiClass> kBidiClassRangeMap = {')
    for (range_beg, range_class) in sorted(class_ranges):
        print(f'    {{ {hex(range_beg)}, {range_class} }},')
    print('};')
    print()


def generate_decomposition_map():
    unicode_data_reader = create_reader(
        UNICODE_DATA_FILE_URL, DATA_FIELD_NAMES)
    mappings = {}
    for record in unicode_data_reader:
        code = int('0x' + record['code'].strip(), 16)
        mapping = record['decomposition_mapping'].strip().split()
        if not mapping:
            continue
        tag = 'None'
        if mapping[0][0] == '<':
            tag = mapping[0]
            assert tag[-1] == '>'
            tag = tag[1].upper() + tag[2:-1]
            chars = mapping[1:]
        else:
            chars = mapping

        mapping = (tag, *chars)
        mappings[code] = mapping

    print(
        'const std::unordered_map<char32_t, Decomposition> kDecompositionMap = {')
    for (code, mapping) in sorted(mappings.items()):
        tag, *chars = mapping
        chars = ''.join('\\U' + ('00000000' + ch)[-8:] for ch in chars)
        print(f'    {{ {hex(code)}, {{ DecompositionTag::{tag}, U"{chars}" }} }},')
    print('};')
    print()


def main():
    print('#include "unicode_data.h"')
    print()
    print('namespace unicpp {')
    print()

    generate_case_maps()
    generate_general_category_map()
    generate_numeric_type_map()
    generate_bidi_class_map()
    generate_decomposition_map()

    print('}  // namespace unicpp')


if __name__ == '__main__':
    main()
