import sys
import os
import re

PROGRAM = "clang-tidy"

def get_filtered_files(root_dir) :
	default_ignored_paths = ['build', 'third-party']
	patterns = (".c", ".cpp", ".cc", ".h", ".hpp", ".hh")
	files = []
	for dir_path, dir_names, file_names in os.walk(root_dir) :
		dir_names[:] = [d for d in dir_names if not re.match(f"({')|('.join(default_ignored_paths)})", d)]
		for f_name in file_names :
			if (f_name.endswith(patterns)) :
				full_path = os.path.join(root_dir, dir_path, f_name)
				files.append(full_path)
	return files

def run_clang_format(files) :
	command = f"{PROGRAM} -format-style=file -header-filter=. -p build"
	exit_code = 0
	for file in files:
		exit_code |= os.system(f"{command} {file}")
	return exit_code

if __name__ == '__main__' :
	root_dir = sys.argv[1]
	files = get_filtered_files(root_dir)
	sys.exit(run_clang_format(files) >> 8)