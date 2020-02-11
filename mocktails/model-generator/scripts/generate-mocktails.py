#!/usr/bin/python3

import argparse
import sys
import os
import subprocess


def get_name(trace_file):
    basename = os.path.basename(trace_file)
    first_dot = basename.index('.')

    return str(basename[:first_dot])


def generate_model(executable, trace_file, output_dir, output_extension, configuration):
    if not os.path.exists(executable):
        sys.exit("Error: could not find executable to create models.")

    output_file = os.path.join(output_dir, "{}.{}".format(get_name(trace_file), output_extension))

    to_run = "{} -i {} -o {} -c {}".format(executable, os.path.abspath(trace_file), os.path.abspath(output_file),
                                           configuration)
    print(to_run)

    log_file = open(output_dir + "/" + get_name(trace_file) + ".log", "w")
    subprocess.run(to_run.split(), stdout=log_file, stderr=log_file)


def is_valid_trace(trace_file):
    if trace_file.endswith('.ptrc') or trace_file.endswith('.ptrc.gz'):
        return True

    return False


def find_trace_files(trace_dir):
    files = []

    for root, dirnames, filenames in os.walk(trace_dir):
        for f in filenames:
            if is_valid_trace(f):
                files.append(os.path.join(root, f))

    return files


def main():
    p = argparse.ArgumentParser(description="Convert each gem5 trace in a directory into HRD models.")
    p.add_argument('-i', '--input-dir', dest='input_dir', default=None)
    p.add_argument('-o', '--output-dir', dest='output_dir', default=None)
    p.add_argument('-c', '--configuration', dest="configuration", default=None)
    p.add_argument('-m', '--max-root-size', dest="size", default=100000)
    p.add_argument('-e', '--exe', dest="exe", default=None)
    p.add_argument('-x', '--output-extension', dest="extension", default="mocktails.gz")

    (args) = p.parse_args()

    if args.input_dir is None:
        sys.exit("Error: no path to traces directory.")

    if args.output_dir is None:
        sys.exit("Error: no path to output directory.")
    else:
        os.makedirs(args.output_dir, exist_ok=True)

    if args.configuration is None:
        sys.exit("Error: no path to configuration file.")

    if args.exe is None:
        sys.exit("Error: no path to executable.")

    trace_files = find_trace_files(args.input_dir)
    for trace_file in trace_files:
        generate_model(args.exe, trace_file, args.output_dir, args.extension, args.configuration)


if __name__ == "__main__":
    main()
