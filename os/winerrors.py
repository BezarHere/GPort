import io
from pathlib import Path
import os, re


ERR_RE = re.compile(r"#define\s*(\w+)\s*(\d+)[Ll]")
FILL_EMPTY_SPACES = False
TARGET_ROW_LENGTH = 40

def prettify_name(text: str):
	ss = io.StringIO()
	last_f = 0
	f = text.find(' ')
	while f != -1:
		ss.write(text[last_f].upper())
		ss.write(text[last_f + 1:f + 1])

		last_f = f + 1
		f = text.find(' ', f + 1)
	ss.write(text[last_f].upper())
	ss.write(text[last_f + 1:])
	return ss.getvalue()


def extract_errors(line: str):
	s = ERR_RE.search(line)
	if s is None:
		return None
	return prettify_name(s[1].lstrip('ERROR_').lower().replace('_', ' ')).replace(' ', ''), int(s[2])

with open("winerrors.txt", 'r') as f:
	text = f.read()

table = []
last_id = 0
for i in text.splitlines():
	j = extract_errors(i)
	if j is None:
		continue

	if FILL_EMPTY_SPACES:
		for k in range(last_id + 1, j[1]):
			table.append((f"UNAMED_ERROR_{k}", k))
	
	if len(j[0]) < TARGET_ROW_LENGTH:
		j = j[0] + ' ' * (TARGET_ROW_LENGTH - len(j[0])), j[1]

	table.append(j)
	last_id = j[1]

with open("winerrors_repr.txt", 'w') as f:
	for i in table:
		f.write(f"{i[0]} = {i[1]},\n")
