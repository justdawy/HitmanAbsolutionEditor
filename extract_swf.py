import sys
import re

def extract_strings(filename):
    with open(filename, 'rb') as f:
        data = f.read()
    
    # Try to find sequences of ASCII characters
    # Font names in DefineFont tags are usually length-prefixed, but the string is just plain ASCII.
    # Actually SWF files might be compressed (CWS instead of FWS).
    # Let's check the signature.
    sig = data[:3]
    if sig == b'CWS':
        import zlib
        # zlib compressed
        data = data[:8] + zlib.decompress(data[8:])
    elif sig == b'FWS':
        pass
    else:
        print(f"{filename} is not a valid SWF")
        return

    # Find printable ASCII strings
    strings = re.findall(b'[a-zA-Z0-9_\- ]{4,}', data)
    print(f"Strings in {filename}:")
    unique_strings = set(s.decode('ascii') for s in strings)
    for s in sorted(unique_strings):
        if "Hitman" in s or "hitman" in s.lower() or "font" in s.lower():
            print("  ", s)

extract_strings(r"C:\Users\justdawy\Desktop\penis\fonts_en_0x80000000D0196476.flashmovie.swf")
extract_strings(r"C:\Users\justdawy\Desktop\penis\fonts_ru_0x80000001B3B7D677CC.flashmovie.swf")
extract_strings(r"C:\Users\justdawy\Desktop\penis\fonts_ru_0x8000000054A5AB810.flashmovie.swf")
