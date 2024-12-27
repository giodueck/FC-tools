import zlib
import base64
import sys
import copy

# Usage: python json-to-template-bp.py <decoded-bp-string.json>
# Output: "out.bp"

argv = sys.argv
filename = argv[1]
with open(filename, "rt") as fd:
    json = fd.read()

oldJson = ""
i = 0
score = 0
while json != oldJson or score <= 1:
    score += json == oldJson
    oldJson = copy.deepcopy(json)
    json = json.replace('"count":1}', '"count":' + str(i + 0x7F000000 - 0x80000000 + 512 * 7) + '}', 1)
    i += 4
    i += i // 512
    i %= 512

json = json.replace(',"group":"RAM Signal Library 128"', '')
deflated = zlib.compress(bytes(json, 'utf-8'), 9)
b64 = base64.b64encode(deflated)

with open("out.bp", "wt") as fd:
    fd.write('0')
    fd.write(str(b64)[2:-1])
