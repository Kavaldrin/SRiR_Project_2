import argparse
import networkx as nx
import matplotlib.pyplot as plt
import numpy as np
import json
import subprocess as sp
import shlex
import os


def make_random_graph(options):
    g = nx.generators.random_regular_graph(options['nodes'] - 1, options['nodes'])

    d = g.to_directed()

    for e in d.edges:
        if not options['symmetrical']:
            d[e[0]][e[1]]['weight'] = options['max_weight'] * np.random.rand()
        else:
            if d.has_edge(e[1], e[0]) and 'weight' in d[e[1]][e[0]]:
                d[e[0]][e[1]]['weight'] = d[e[1]][e[0]]['weight']
            else:
                d[e[0]][e[1]]['weight'] = (options['max_weight'] - options['min_weight']) * np.random.rand() + options['min_weight']

    if options['debug']:
        for i in range(len(d.nodes) - 1):
            d[i][i + 1]['weight'] = 1.0
        d[len(d.nodes) - 1][0]['weight'] = 1.0

    return d


def save_to_file(data, options):
    with open(options['output'] + '.g', 'w') as file:
        json.dump(json.loads(data), file, indent=4)


def load_graph_file(options):
    with open(options['output'] + '.g', 'r') as file:
        content = json.load(file)
        d = nx.jit_graph(content, create_using=nx.DiGraph())
        return d


def save_res(options, tsp):
    d = load_graph_file(options)
    pos = {n[0]: np.array([n[1]['pos'][0], n[1]['pos'][1]]) for n in d.nodes(data=True)}
    clean_d = nx.DiGraph()
    clean_d.add_nodes_from(d.nodes)

    edge_labels = {}

    if not tsp and options['save_result'] is not None:
        for i in range(len(options['save_result']) - 1):
            clean_d.add_edge(int(options['save_result'][i]), int(options['save_result'][i + 1]))
            edge_labels[(int(options['save_result'][i]), int(options['save_result'][i + 1]),)] = round(d[int(options['save_result'][i])][int(options['save_result'][i + 1])]['weight'], 2)
    elif tsp is not None:
        for i in range(len(tsp) - 1):
            clean_d.add_edge(tsp[i], tsp[i + 1])
            edge_labels[(tsp[i], tsp[i + 1], )] = round(d[tsp[i]][tsp[i + 1]]['weight'], 2)

    if len(edge_labels):
        plt.clf()
        nx.draw(clean_d, pos=pos, with_labels=True)
        nx.draw_networkx_edge_labels(clean_d, pos, edge_labels=edge_labels, label_pos=0.2, font_size=7)
        plt.savefig(options['output'] + '_res.png')


def run_solver(options):
    nodes = sp.check_output(['upcxx-nodes', options['compute_nodes']], text=True)
    run_cmd = 'upcxx-run -n ' + str(options['processes']) + ' ' + nodes  + ' ./main_program/antColonyTSPSolver ' + os.path.abspath(options['output'] + '.g')
    print('Running: ' + run_cmd)
    out = sp.check_output(shlex.split(run_cmd), text=True)
    print('---------- PROGRAM OUTPUT ----------')
    print(out)
    print('---------- PROGRAM OUTPUT ----------')
    route = out.split('\n')[0]
    route = route.split()[1:]
    route = [int(el) for el in route]
    return route


def main():
    parser = argparse.ArgumentParser(description='Simple script for running project.')

    parser.add_argument('-n', '--nodes', help='Amount of nodes in the graph to be generated', type=int, default=10)
    parser.add_argument('-max', '--max_weight', help='Max weight of edge in generated graph', type=float, default=200)
    parser.add_argument('-min', '--min_weight', help='Min weight of edge in generated graph', type=float, default=10)
    parser.add_argument('-s', '--symmetrical', help='Switch to make the graph for symmetrical or asymmetrical TSP problem. Default is asymmetrical', action='store_true')
    parser.add_argument('-o', '--output', help='Path or filename where to generate the graph file', type=str, default='./generated_graph')
    parser.add_argument('-d', '--debug', help='Switch whether to make graph for debugging that is to make the best path be consecutive nodes 0 - n with weights of 1.0', action='store_true')
    parser.add_argument('-r', '--run', help='Switch whether to run ant colony TSP MPI solver', action='store_true')
    parser.add_argument('-cn', '--compute_nodes', help='Path to "nodes" file', type=str, default='./nodes')
    parser.add_argument('-p', '--processes', help='Amount of processes to be launched on each computing node', type=int, default=1)
    parser.add_argument('-dg', '--dont_generate', help='Switch whether to generate a random graph', action='store_true')
    parser.add_argument('-sr', '--save_result', help='Pass array of integers (TSP path, result of main program) to generate result graph', nargs='*')

    options = vars(parser.parse_args())
    res = None
    if not options['dont_generate']:
        d = make_random_graph(options)
        pos = nx.spring_layout(d)
        edge_labels = dict([((u, v,), round(w['weight'], 2)) for u, v, w in d.edges(data=True)])
        nx.draw_networkx_edge_labels(d, pos, edge_labels=edge_labels, label_pos=0.2, font_size=7, bbox={'alpha': 0})
        nx.draw(d, pos, with_labels=True, connectionstyle='arc3, rad=0.1')
        for i in range(len(d.nodes)):
            d.nodes[i]['pos'] = list(pos[i])
        res = nx.jit_data(d)
        save_to_file(res, options)
        plt.savefig(options['output'] + '.png')

    tsp = None
    if options['run']:
        tsp = run_solver(options)

    save_res(options, tsp)


if __name__ == "__main__":
    main()
