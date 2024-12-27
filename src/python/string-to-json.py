import zlib
import base64
import sys

# Usage: python string-to-json.py <import-string.txt>
# Return: "out.json"

argv = sys.argv
filename = argv[1]
with open(filename, "rt") as fd:
    b64_string = fd.read()

b64_string = b64_string[1:]
bin_string = base64.b64decode(bytes(b64_string, 'utf-8'))

json = zlib.decompress(bin_string)

with open("out.json", "wt") as fd:
    fd.write(str(json)[2:-1])
