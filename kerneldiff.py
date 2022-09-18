#!/usr/bin/env python3

import os
import sys

if __name__ == "__main__":
    args = sys.argv
    if len(args) < 4:
        print("Usage: kernelcache_raw kernelcache_patched kc.bpatch")
        sys.exit(0)

    patched = args[2]
    original = args[1]
    sizeP = os.path.getsize(patched)
    sizeO = os.path.getsize(original)

    if sizeP != sizeO:
        print("size does not match, can't compare files! exiting...")
        sys.exit(1)

    p = open(patched, "rb").read()
    o = open(original, "rb").read()

    a = 0
    diff = []
    diffFile = open(args[3], 'w+')
    diffFile.write('#AMFI\n\n')
    for i in range(sizeO):
        originalByte = o[i]
        patchedByte = p[i]
        if originalByte != patchedByte:
            if a == 1e6:
                for d in diff:
                    data = str(d[0]) + " " + (str(d[1])) + " " + (str(d[2]))
                    diffFile.write(data + '\n')
                diff = []
                a = 0
            diff.append([hex(i), hex(originalByte), hex(patchedByte)])
            a += 1

    for d in diff:
        data = str(d[0]) + " " + (str(d[1])) + " " + (str(d[2]))
        diffFile.write(data + '\n')
