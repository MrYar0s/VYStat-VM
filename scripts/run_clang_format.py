import sys
import os
import re

PROGRAM = "clang-format"

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
	command = f"{PROGRAM} -i"
	for file in files:
		os.system(f"{command} {file}")

if __name__ == '__main__' :
	root_dir = sys.argv[1]
	files = get_filtered_files(root_dir)
	run_clang_format(files)