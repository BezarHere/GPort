from functools import cache
import sys, os, glob
from pathlib import Path
from typing import Callable, Iterable

__folder__ = Path(__file__).parent

DEFAULT_ARGS_PREFIX = 'gcc', '-fanalyzer', '-std=c2x', '-Wall'
DEFAULT_ARGS_SUFFIX_NON_DIS = '-o', 'main.exe'
DEFAULT_ARGS_SUFFIX_DIS = '-o', 'main.asm'
OUTPUT_EXC_FILE = ('.\\main.exe', )

RELEASE_BUILD_ARGS = ('-O3', )
DEBUG_BUILD_ARGS = ('-ggdb3', '-g3')

_BUILD_TYPES: dict[str, Callable] = {}
_BUILD_TYPE_DEFAULT: str = ''

postrun_enabled = False

def run(args):
	command = args if isinstance(args, str) else ' '.join(map(str, args))
	print(f"\t[ EXCUTING: {command} ]")
	os.system(command)

@cache
def _most_similar_impl(base: str, cands: Iterable[str], *, default: str | None = None):
	if isinstance(cands, (str, bytes)):
		raise ValueError("Can't iterate of str/bytes")
	cands = tuple(cands)
	lengths = tuple((k, len(k)) for k in cands)
	sim_length = {k: 0 for k in cands}
	skiped_types = []


	for i, v in enumerate(base):

		for j, k in enumerate(lengths):
			if k[1] <= i:
				skiped_types.append(j)
				continue

			if k[0][i] == v:
				sim_length[k[0]] += 1
			else:
				skiped_types.append(j)
		
		if skiped_types:	
			for n in skiped_types:
				lengths = lengths[:n] + lengths[n + 1:]
			skiped_types.clear()
		
	
	max_similarity = 0
	max_sim_name = ''
	for i, n in lengths:
		if sim_length[i] > max_similarity:
			max_similarity = sim_length[i]
			max_sim_name = i
	
	if max_similarity == 0 or max_sim_name == '':
		max_sim_name = default
	return max_sim_name

def most_similar(base: str, cands: Iterable[str], *, default: str | None = None):
	cands = tuple(cands)
	return _most_similar_impl(base, cands, default=default)

def build_method(name: str, *, default: bool = False):
	def wrapper(f: Callable):
		_BUILD_TYPES[name] = f
		if default:
			_BUILD_TYPE_DEFAULT = name
		def function(*args, **kwargs):
			return f(*args, **kwargs)
	return wrapper

def get_trans_files():
	return tuple(__folder__.glob(r'**\*.c'))


@build_method('release')
def release_build():
	run(DEFAULT_ARGS_PREFIX + RELEASE_BUILD_ARGS + get_trans_files() + DEFAULT_ARGS_SUFFIX_NON_DIS)

@build_method('debug', default=True)
def debug_build():
	run(DEFAULT_ARGS_PREFIX + DEBUG_BUILD_ARGS + get_trans_files() + DEFAULT_ARGS_SUFFIX_NON_DIS)

@build_method('disassemply')
def disassemply_build(args: list[str] | None = None):
	if args is None or len(args) == 0:
		raise ValueError('Please pass a source filepath/name to disassemple.')
	files = get_trans_files()

	if args[0].lower()[:4] == '-all':
		print("\t[ DISASSEMPLING ALL ]")
		for i in files:
			print(f"\t[ DISASSEMPLING FILE: \"{i.relative_to(__folder__)}\" ]")
			run(DEFAULT_ARGS_PREFIX + DEBUG_BUILD_ARGS + ('-S', f"\"{i.absolute()}\"") + DEFAULT_ARGS_SUFFIX_DIS)
		return

	filepath = most_similar(str(Path(args[0]).resolve().absolute()), map(lambda p: str(p.resolve().absolute()), files))
	filepath = Path(filepath).resolve()

	if not filepath.exists():
		raise FileNotFoundError(f"Can't find source file: \"{filepath}\"")

	print(f"\t[ DISASSEMPLING FILE: \"{filepath.relative_to(__folder__)}\" ]")
	run(DEFAULT_ARGS_PREFIX + DEBUG_BUILD_ARGS + ('-S', f"\"{filepath}\"") + DEFAULT_ARGS_SUFFIX_DIS)




def main():
	global postrun_enabled
	build_name = sys.argv[1].lower() if len(sys.argv) > 1 else _BUILD_TYPE_DEFAULT
	build_name = most_similar(build_name, _BUILD_TYPES.keys(), default=_BUILD_TYPE_DEFAULT)
	

	unused_args = []
	for i in sys.argv[2:]:
		if i.lower()[:4] == '-run':
			postrun_enabled = True
			continue
		
		unused_args.append(i)

	print(f"\t[ BUILDING {build_name} ]".upper())
	if len(unused_args) == 0:
		_BUILD_TYPES[build_name]()
	else:
		_BUILD_TYPES[build_name](unused_args)
	

	if postrun_enabled:
		run(OUTPUT_EXC_FILE)


if __name__ == '__main__':
	main()
	
