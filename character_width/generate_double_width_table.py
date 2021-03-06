import sys
import argparse

import parse_eaw
import range_list


def eaw_to_width(eaw):
    if eaw == 'W':  return 2
    if eaw == 'F':  return 2
    if eaw == 'Na': return 1
    if eaw == 'H':  return 1
    if eaw == 'N':  return 1
    if eaw == 'A':  return 1
    raise Exception(f'Unhandled Value {eaw}')


parser = argparse.ArgumentParser(description='Generate double width table')
parser.add_argument('--input', required=True)
parser.add_argument('--output', required=True)

args = parser.parse_args()

east_asian_width = parse_eaw.parse(args.input)
width_table = range_list.transform_list(east_asian_width, eaw_to_width)

with open(args.output, 'w') as output:
    print(f'// This file was generated by {__file__}', file=output);
    count = 0
    for w in list(width_table):
        if w.property == 2:
            print(f'{{ {w.range_start:#7x}, {w.range_end:#7x} }},', end='', file=output)

            count += 1
            if count % 3 == 0:
                print(end='\n', file=output)
            else:
                print(' ', end='', file=output)

    print(end='\n', file=output)
