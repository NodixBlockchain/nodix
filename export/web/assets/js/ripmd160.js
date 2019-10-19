
var RMDsize = 160;
 var X = new Array();

 function ROL(x, n) {
 	return new Number((x << n) | (x >>> (32 - n)))
 }

 function F(x, y, z) {
 	return new Number(x ^ y ^ z)
 }

 function G(x, y, z) {
 	return new Number((x & y) | (~x & z))
 }

 function H(x, y, z) {
 	return new Number((x | ~y) ^ z)
 }

 function I(x, y, z) {
 	return new Number((x & z) | (y & ~z))
 }

 function J(x, y, z) {
 	return new Number(x ^ (y | ~z))
 }

 function mixOneRound(a, b, c, d, e, x, s, f) {
 	switch (f) {
 		case 0:
 			a += F(b, c, d) + x + 0x00000000;
 			break;
 		case 1:
 			a += G(b, c, d) + x + 0x5a827999;
 			break;
 		case 2:
 			a += H(b, c, d) + x + 0x6ed9eba1;
 			break;
 		case 3:
 			a += I(b, c, d) + x + 0x8f1bbcdc;
 			break;
 		case 4:
 			a += J(b, c, d) + x + 0xa953fd4e;
 			break;
 		case 5:
 			a += J(b, c, d) + x + 0x50a28be6;
 			break;
 		case 6:
 			a += I(b, c, d) + x + 0x5c4dd124;
 			break;
 		case 7:
 			a += H(b, c, d) + x + 0x6d703ef3;
 			break;
 		case 8:
 			a += G(b, c, d) + x + 0x7a6d76e9;
 			break;
 		case 9:
 			a += F(b, c, d) + x + 0x00000000;
 			break;
 		default:
 			document.write("Bogus round number");
 			break
 	}
 	a = ROL(a, s) + e;
 	c = ROL(c, 10);
 	a &= 0xffffffff;
 	b &= 0xffffffff;
 	c &= 0xffffffff;
 	d &= 0xffffffff;
 	e &= 0xffffffff;
 	var g = new Array();
 	g[0] = a;
 	g[1] = b;
 	g[2] = c;
 	g[3] = d;
 	g[4] = e;
 	g[5] = x;
 	g[6] = s;
 	return g
 }

 function MDinit(a) {
 	a[0] = 0x67452301;
 	a[1] = 0xefcdab89;
 	a[2] = 0x98badcfe;
 	a[3] = 0x10325476;
 	a[4] = 0xc3d2e1f0
 }
 var ROLs = [
 	[11, 14, 15, 12, 5, 8, 7, 9, 11, 13, 14, 15, 6, 7, 9, 8],
 	[7, 6, 8, 13, 11, 9, 7, 15, 7, 12, 15, 9, 11, 7, 13, 12],
 	[11, 13, 6, 7, 14, 9, 13, 15, 14, 8, 13, 6, 5, 12, 7, 5],
 	[11, 12, 14, 15, 14, 15, 9, 8, 9, 14, 5, 6, 8, 6, 5, 12],
 	[9, 15, 5, 11, 6, 8, 13, 12, 5, 12, 13, 14, 11, 8, 5, 6],
 	[8, 9, 9, 11, 13, 15, 15, 5, 7, 7, 8, 11, 14, 14, 12, 6],
 	[9, 13, 15, 7, 12, 8, 9, 11, 7, 7, 12, 7, 6, 15, 13, 11],
 	[9, 7, 15, 11, 8, 6, 6, 14, 12, 13, 5, 14, 13, 13, 7, 5],
 	[15, 5, 8, 11, 14, 14, 6, 14, 6, 9, 12, 9, 12, 5, 15, 8],
 	[8, 5, 12, 9, 12, 5, 14, 6, 8, 13, 6, 5, 15, 13, 11, 11]
 ];
 var indexes = [
 	[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15],
 	[7, 4, 13, 1, 10, 6, 15, 3, 12, 0, 9, 5, 2, 14, 11, 8],
 	[3, 10, 14, 4, 9, 15, 8, 1, 2, 7, 0, 6, 13, 11, 5, 12],
 	[1, 9, 11, 10, 0, 8, 12, 4, 13, 3, 7, 15, 14, 5, 6, 2],
 	[4, 0, 5, 9, 7, 12, 2, 10, 14, 1, 3, 8, 11, 6, 15, 13],
 	[5, 14, 7, 0, 9, 2, 11, 4, 13, 6, 15, 8, 1, 10, 3, 12],
 	[6, 11, 3, 7, 0, 13, 5, 10, 14, 15, 8, 12, 4, 9, 1, 2],
 	[15, 5, 1, 3, 7, 14, 6, 9, 11, 8, 12, 2, 10, 0, 4, 13],
 	[8, 6, 4, 1, 3, 11, 15, 0, 5, 12, 2, 13, 9, 7, 10, 14],
 	[12, 15, 10, 4, 1, 5, 8, 7, 6, 2, 13, 14, 0, 3, 9, 11]
 ];

 function compress(a, X) {
 	blockA = new Array();
 	blockB = new Array();
 	var b;
 	for (var i = 0; i < 5; i++) {
 		blockA[i] = new Number(a[i]);
 		blockB[i] = new Number(a[i])
 	}
 	var c = 0;
 	for (var j = 0; j < 5; j++) {
 		for (var i = 0; i < 16; i++) {
 			b = mixOneRound(blockA[(c + 0) % 5], blockA[(c + 1) % 5], blockA[(c + 2) % 5], blockA[(c + 3) % 5], blockA[(c + 4) % 5], X[indexes[j][i]], ROLs[j][i], j);
 			blockA[(c + 0) % 5] = b[0];
 			blockA[(c + 1) % 5] = b[1];
 			blockA[(c + 2) % 5] = b[2];
 			blockA[(c + 3) % 5] = b[3];
 			blockA[(c + 4) % 5] = b[4];
 			c += 4
 		}
 	}
 	c = 0;
 	for (var j = 5; j < 10; j++) {
 		for (var i = 0; i < 16; i++) {
 			b = mixOneRound(blockB[(c + 0) % 5], blockB[(c + 1) % 5], blockB[(c + 2) % 5], blockB[(c + 3) % 5], blockB[(c + 4) % 5], X[indexes[j][i]], ROLs[j][i], j);
 			blockB[(c + 0) % 5] = b[0];
 			blockB[(c + 1) % 5] = b[1];
 			blockB[(c + 2) % 5] = b[2];
 			blockB[(c + 3) % 5] = b[3];
 			blockB[(c + 4) % 5] = b[4];
 			c += 4
 		}
 	}
 	blockB[3] += blockA[2] + a[1];
 	a[1] = a[2] + blockA[3] + blockB[4];
 	a[2] = a[3] + blockA[4] + blockB[0];
 	a[3] = a[4] + blockA[0] + blockB[1];
 	a[4] = a[0] + blockA[1] + blockB[2];
 	a[0] = blockB[3]
 }

 function zeroX(X) {
 	for (var i = 0; i < 16; i++) {
 		X[i] = 0
 	}
 }

 function MDfinish(a, b, c, d) {
 	var X = new Array(16);
 	zeroX(X);
 	var j = 0;
 	for (var i = 0; i < (c & 63); i++) {
 		X[i >>> 2] ^= (b.charCodeAt(j++) & 255) << (8 * (i & 3))
 	}
 	X[(c >>> 2) & 15] ^= 1 << (8 * (c & 3) + 7);
 	if ((c & 63) > 55) {
 		compress(a, X);
 		var X = new Array(16);
 		zeroX(X)
 	}
 	X[14] = c << 3;
 	X[15] = (c >>> 29) | (d << 3);
 	compress(a, X)
 }

 function BYTES_TO_DWORD(a) {
 	var b = (a.charCodeAt(3) & 255) << 24;
 	b |= (a.charCodeAt(2) & 255) << 16;
 	b |= (a.charCodeAt(1) & 255) << 8;
 	b |= (a.charCodeAt(0) & 255);
 	return b
 }

 function RMD(a) {
 	var b = new Array(RMDsize / 32);
 	var c = new Array(RMDsize / 8);
 	var d;
 	var e;
 	MDinit(b);
 	d = a.length;
 	var X = new Array(16);
 	zeroX(X);
 	var j = 0;
 	for (var e = d; e > 63; e -= 64) {
 		for (var i = 0; i < 16; i++) {
 			X[i] = BYTES_TO_DWORD(a.substr(j, 4));
 			j += 4
 		}
 		compress(b, X)
 	}
 	MDfinish(b, a.substr(j), d, 0);
 	for (var i = 0; i < RMDsize / 8; i += 4) {
 		c[i] = b[i >>> 2] & 255;
 		c[i + 1] = (b[i >>> 2] >>> 8) & 255;
 		c[i + 2] = (b[i >>> 2] >>> 16) & 255;
 		c[i + 3] = (b[i >>> 2] >>> 24) & 255
 	}
 	return c
 }

 function toHex32(x) {
 	var a = "0123456789abcdef";
 	var b = "";
 	for (var i = 0; i < 2; i++) {
 		b = String(a.charAt(x & 0xf)).concat(b);
 		x >>>= 4
 	}
 	return b
 }

 function toRMDstring(a) {
 	var b = "";
 	for (var i = 0; i < RMDsize / 8; i++) {
 		b += toHex32(a[i])
 	}
 	return b
 }

 function RMDstring(a) {
 	var b = RMD(a);
 	var c = "";
 	for (var i = 0; i < RMDsize / 8; i++) {
 		c += toHex32(b[i])
 	}
 	return c
 }