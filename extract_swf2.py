import sys
import re
import os

def extract_strings(filename):
    with open(filename, 'rb') as f:
        data = f.read()
    
    sig = data[:3]
    if sig == b'CWS':
        import zlib
        data = data[:8] + zlib.decompress(data[8:])
    elif sig == b'FWS':
        pass
    else:
        return

    strings = re.findall(br'[a-zA-Z0-9_\- ]{4,}', data)
    print(f"\nStrings in {os.path.basename(filename)}:")
    unique_strings = set(s.decode('ascii') for s in strings)
    for s in sorted(unique_strings):
        if "Hitman" in s or "hitman" in s.lower() or "font" in s.lower():
            print("  ", s)

directory = r"C:\Users\justdawy\Desktop\penis"
for file in os.listdir(directory):
    if file.endswith(".swf"):
        extract_strings(os.path.join(directory, file))
