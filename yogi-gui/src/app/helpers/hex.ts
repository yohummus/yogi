export function hexStringToByteBuffer(hex: string): ByteBuffer {
  let byteStrings = hex.toUpperCase().split(' ').filter((byteString) => {
    return byteString.length > 0;
  });

  let strictHex = '';
  for (let s of byteStrings) {
    for (let i = 0; i < s.length; ++i) {
      let code = s.charCodeAt(i);
      if (i > 1 || code < 48 || code > 70 || (code < 65 && code > 57)) {
        throw new SyntaxError('Not a valid HEX string');
      }
    }

    if (s.length === 1) {
      strictHex += '0';
    }

    strictHex += s;
  }

  return ByteBuffer.fromHex(strictHex);
}

export function byteBufferToHexString(buf: ByteBuffer): string {
  let hex: string = buf.toHex();

  let hexStr = '';
  for (let i = 0; i < hex.length; i += 2) {
    hexStr += hex.substr(i, 2) + ' ';
  }

  return hexStr.substr(0, hexStr.length - 1);
}
