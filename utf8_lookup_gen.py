


import itertools


def char_len(byte: int):
	if byte < (3 << 6):
		return 1
	b = bin(byte)[2:]
	if len(b) < 8:
		b = '0' * (8 - len(b)) + b
	x = 0
	for i in b:
		if i == '1':
			x += 1
			continue
		break
	return x


out_str = ''
for i in itertools.batched(range(256), 16):
	for j in i:
		out_str += f"{char_len(j)}, "
	out_str += '\n'
print(out_str)
