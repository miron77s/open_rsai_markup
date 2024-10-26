import os
import re
import argparse

# Create the parser
parser = argparse.ArgumentParser(description="Process some paths.")

# Add the arguments to the parser
parser.add_argument('--source_dir_path', required=True, help='Path to the Bing directory')
parser.add_argument('--target_dir_path', required=True, help='Path to another directory')
parser.add_argument('--markup_path', required=True, help='Path to the markup file')

# Parse the arguments
args = parser.parse_args()

bing_dir_path = args.source_dir_path
another_dir_path = args.target_dir_path
markup_path = args.markup_path

# Check if the paths are set correctly and exist
if not os.path.isdir(bing_dir_path):
    print(f"Error: The Bing directory path '{bing_dir_path}' does not exist or is not a directory.")
    exit(1)

if not os.path.isdir(another_dir_path):
    print(f"Error: The another directory path '{another_dir_path}' does not exist or is not a directory.")
    exit(1)

if not os.path.isfile(markup_path):
    print(f"Error: The markup file path '{markup_path}' does not exist or is not a file.")
    exit(1)

# If all checks pass, continue with the rest of the script
print("Source path '", bing_dir_path, "' :")
bing_list = os.listdir(bing_dir_path)
# for path in bing_list :
#     print(path)

print("Target path '", another_dir_path, "' :")
another_list = os.listdir(another_dir_path)
# for path in another_list :
#     print(path)

bing_dict = {}
for bing_path in bing_list :
    bing_key =  re.sub(".*" + os.path.basename(bing_dir_path), '', bing_path)
    bing_key =  re.sub(".jpg", '', bing_key)
    bing_dict[bing_key] = bing_path

path_dict = {}
for another_path in another_list :
    another_key =  re.sub(".*" + os.path.basename(another_dir_path), '', another_path)
    another_key =  re.sub(".png", '', another_key)
    # print(another_key)
    status = False
    for bing_key, bing_path in bing_dict.items():
        if another_key == bing_key :
            #print(another_key, ' <->', bing_key)
            path_dict[another_path] = bing_path
            status = True
            break
    #if status == False :
        #print(another_key, ' ERROR')
        # break

f = open(markup_path, "r")
markup = f.read()

for another_path, bing_path in path_dict.items():
    if re.search(bing_path, markup) :
        print(another_path, ' <->', bing_path)
        markup =  re.sub(bing_path, another_path, markup)

new_markup_path = os.path.basename(another_dir_path) + '_' + os.path.basename(markup_path)

f = open(new_markup_path, "a")
f.write(markup)
f.close()


