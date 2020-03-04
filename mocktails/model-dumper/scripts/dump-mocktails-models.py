#!/usr/bin/python3

import argparse
import sys
import os
import subprocess


def get_name(model_file):
    basename = os.path.basename(model_file)
    first_dot = basename.index('.')

    return str(basename[:first_dot])


def dump_model(executable, model_file, output_dir, output_extension):
    if not os.path.exists(executable):
        sys.exit("Error: could not find executable to dump models.")

    output_file = os.path.join(output_dir, "{}.{}".format(get_name(model_file), output_extension))

    to_run = "{} -i {} -o {}".format(executable, os.path.abspath(model_file), os.path.abspath(output_file))
    print(to_run)

    log_file = open(output_dir + "/" + get_name(model_file) + ".log", "w")
    subprocess.run(to_run.split(), stdout=log_file, stderr=log_file)


def is_valid_model(model_file):
    if model_file.endswith('.mocktails') or model_file.endswith('.mocktails.gz'):
        return True

    return False


def find_model_files(model_dir):
    files = []

    for root, dirnames, filenames in os.walk(model_dir):
        for f in filenames:
            if is_valid_model(f):
                files.append(os.path.join(root, f))

    return files


def main():
    p = argparse.ArgumentParser(description="Dump each HRD model in a directory into csv files.")
    p.add_argument('-i', '--input-dir', dest='input_dir', default=None)
    p.add_argument('-o', '--output-dir', dest='output_dir', default=None)
    p.add_argument('-e', '--exe', dest="exe", default=None)
    p.add_argument('-x', '--output-extension', dest="extension", default="csv")

    (args) = p.parse_args()

    if args.input_dir is None:
        sys.exit("Error: no path to models directory.")

    if args.output_dir is None:
        sys.exit("Error: no path to output directory.")
    else:
        os.makedirs(args.output_dir, exist_ok=True)

    if args.exe is None:
        sys.exit("Error: no path to executable.")

    model_files = find_model_files(args.input_dir)
    for model_file in model_files:
        dump_model(args.exe, model_file, args.output_dir, args.extension)


if __name__ == "__main__":
    main()
