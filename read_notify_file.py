#! /usr/bin/env python

import base64

with open('test.nf', 'r') as f:
    for l in f:
        ll = l.split(' ', 1)
        print("--- type: " + ll[0])
        print("--- data: " + ll[1].strip())
        coded_string = ll[1].strip()
        print("--- value: " + base64.b64decode(coded_string))

