# -*- encoding=utf-8 -*- 

import os
import sys
import csv
import shutil


def process_properties(properties_file, results_file, destination_path):
    with open(properties_file, 'r') as file:
        content = file.readlines()

    header = ['Test items', 'Test result']
    items = []
    results = []
    pass_status = True
    print_content = False

    with open(results_file, 'wb') as file:
        writer = csv.writer(file)
        writer.writerow(header)

        for line in content:
            if 'STEP_RESULT' in line:
                items.append(line.split('_STEP_RESULT')[0].lower())
                result_value = line.split('=')[-1].strip()
                if result_value == '0':
                    results.append('pass')
                else:
                    results.append('fail')
                    pass_status = False

            if line.startswith(' ----'):
                print_content = True
                print(line.strip())
                continue

            if print_content:
                print(line.strip())

        writer.writerows(zip(items, results))

    if pass_status:
        print("FINAL Result: PASS")
        return 0
    else:
        print("FINAL Result: FAIL")
        return -1

def copy_files(properties_file, results_file, destination_path):
    try:
        if not os.path.exists(destination_path):
            os.makedirs(destination_path)

        shutil.copy(properties_file, destination_path)
        shutil.copy(results_file, destination_path)
        print("Files copied successfully to {}".format(destination_path))
    except Exception as e:
        print("Error copying files: {}".format(e))
        return -1

if __name__ == '__main__':
    properties_file = 'properties.properties'
    results_file = 'results.csv'
    destination_path = sys.argv[1]

    exit_code = process_properties(properties_file, results_file, destination_path)

    copy_files(properties_file, results_file, destination_path)

    sys.exit(exit_code)