#! /usr/bin/env python3

import yaml
import sys

if len(sys.argv) != 4:
    print("Usage: %s input output version" % sys.argv[0])
    sys.exit(1)

input_file = sys.argv[1]
output_file = sys.argv[2]
version = sys.argv[3]

f77 = None
fc = None
config = None

with open(input_file, 'r') as file:
    config = yaml.load(file, Loader=yaml.FullLoader)

    for e in config['compilers']:
        if version in e['compiler']['spec']:
            f77 = e['compiler']['paths']['f77']
            fc = e['compiler']['paths']['fc']

    for e in config['compilers']:
        if "clang" in e['compiler']['spec']:
            e['compiler']['paths']['f77'] = f77
            e['compiler']['paths']['fc'] = fc

with open(output_file, 'w') as file:
    yaml.dump(config, file)
