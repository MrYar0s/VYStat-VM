import os
import sys

def clone_repo(line_from_file, third_party_path) :
	parsed_string = line_from_file.split(",")
	repo_name = parsed_string[0]
	repo_link = parsed_string[1]
	repo_branch = parsed_string[2]
	if (not os.path.exists(third_party_path+"/"+repo_name)):
		print(f"Clonning {repo_name} in {third_party_path}")
		os.system(f"git clone --verbose {repo_link} -b {repo_branch} --depth=1 {third_party_path}/{repo_name}")

if __name__ == '__main__' :
	list_path = sys.argv[1]
	third_party_path = sys.argv[2]
	file = open(list_path, 'r')
	text = file.readlines()
	for line in text :
		clone_repo(line, third_party_path)