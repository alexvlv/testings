#!/usr/bin/env python3
# GIT Rev.: $Format:%cd %cn %h %D$

import sys

chunk = bytes(range(256))
while True:
    sys.stdout.buffer.write(chunk)
