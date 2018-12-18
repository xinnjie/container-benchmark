import matplotlib
matplotlib.use('agg')
import matplotlib.pyplot as plt
import numpy as np


def parse_benchmark_from_file(filepath):
    benchmarks = {}
    with open(filepath) as benchmark_file:
        for line in benchmark_file:
            tokens = line.split()
            benchmark_case = tokens[0].split('/')[0]
            benchmark_scale = int(tokens[0].split('/')[1])
            benchmark_consume_time = int(tokens[1])


            if not benchmarks.get(benchmark_case):
                benchmarks[benchmark_case] = []
            benchmarks[benchmark_case].append((benchmark_scale, benchmark_consume_time))
    return benchmarks


def plot(benchmarks):
    plt.xlabel('map元素数量')
    plt.ylabel('消耗时间/ns')
    for key in benchmarks:
            x = [item[0] for item in benchmarks[key]]
            y = [item[1] for item in benchmarks[key]]
            plt.plot(x, y, label=key)
    plt.legend()
    print(benchmarks)


if __name__  == '__main__':
    plt.rcParams['font.sans-serif'] = ['Arial Unicode MS']
    benchmarks = parse_benchmark_from_file("benchmark_output_map_lookup.txt")
    plot(benchmarks)
    plt.savefig('benchmark-chart-map_lookup.png')
