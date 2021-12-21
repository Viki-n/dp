import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from math import log2

LABELS = {
    'touch': 'Dotčené vrcholy na operaci',
    'time': 'Průměrný čas na operaci [ms]',
    'rb': 'Červenočerný',
    'splay': 'Splay',
    'multisplay': 'Multisplay',
    'tango': 'Tango'
}

COLORS = {
'rb': 'tab:orange',
    'splay': 'tab:green',
    'multisplay': 'tab:blue',
    'tango': 'tab:red'
}

def load_data(path):
    line_data = None
    data = []
    for i, line in enumerate(open('../data/'+path, 'r')):
        if i % 2 == 0:
            line = line.split()
            line_data = {
                'structure': line[0].split('_')[1],
                "value_type": line[0].split('_')[2],
                'tree_size': int(line[1]),
                'sequence_length': int(line[2]),
                'sequence_type': line[4],
                'subset_size': None if line[4] in 'sr' else line[5],
                }
        else:
            line = line.split()
            line_data['value'] = int(line[1]) if line_data["value_type"] == 'touch' else float(line[3])
            data.append(line_data)

    df = pd.DataFrame(data)
    print(df)

    return df


def by_one_seq_graphs(prints=False):
    df = load_data('data_by_one')

    df['ratio'] = df.sequence_length / df.tree_size

    for type_ in ['touch']:#, ('time', 'ms per find')]:
        label = LABELS[type_]
        df1 = df.loc[(df['value_type'] == type_) & (df['sequence_type'] == 's')]

        df1_ = df1.loc[df1['ratio'] == 2].copy()
        df1_['value'] -= df1.loc[df1['ratio'] == 1]['value'].values
        df1_['value'] /= df1_['sequence_length']/2

        plt.semilogx()
        for st, aux in df1_.groupby('structure'):
            plt.plot(aux['tree_size'], aux['value'], label=LABELS[st], color=COLORS[st])
            if prints:
                mins = []
                maxes = []
                for i in range(1, len(aux)-1):

                    if aux['value'].iloc[i] > aux['value'].iloc[i+1] and aux['value'].iloc[i] > aux['value'].iloc[i-1]:
                        maxes.append(aux['tree_size'].iloc[i])
                    if aux['value'].iloc[i] < aux['value'].iloc[i + 1] and aux['value'].iloc[i] < aux['value'].iloc[i - 1]:
                        mins.append(aux['tree_size'].iloc[i])

                print(st)
                print('mins:', mins)
                print('maxes:', maxes)
        plt.xlabel('Počet vrcholů stromu')
        plt.ylabel(label)
        plt.legend()
        plt.savefig('../text/graphs/one_by_one_seq.pdf')
        plt.figure()


def simple_graphs(seq_type, prints=False):
    df = load_data('data')
    df['value'] /= df['sequence_length']
    df = df.loc[df['sequence_type'] == seq_type].copy()

    for type_ in ['touch', 'time']:

        df_ = df.loc[df['value_type'] == type_].copy()

        label = LABELS[type_]
        plt.semilogx()

        sizes = []
        data = {}
        for st, aux in df_.groupby('structure'):
            plt.plot(aux['tree_size'], aux['value'], label=LABELS[st], color=COLORS[st])
            data[st] = aux['value'].values
            sizes = sizes or list(aux['tree_size'])

        if prints:
            if type_ == 'touch' and seq_type == 'r':
                logs = np.array(list(map(log2, sizes)))
                plt.plot(sizes, logs, color='gray', label='log(n)')

                for d in data:
                    print(d)
                    print(np.mean(data[d] - logs[:len(data[d])]))
                    print(data[d] / logs[:len(data[d])])

                for d in data:
                    print(d)
                    print(np.mean(data[d]), np.mean(data[d][len(data[d])//2:]))

        if type_ == 'touch' and seq_type == 'r':
            logs = np.array(list(map(log2, sizes)))
            plt.plot(sizes, logs, color='gray', label='log(n)')

        plt.xlabel('Počet vrcholů stromu')
        plt.ylabel(label)
        plt.legend()
        plt.savefig(f'../text/graphs/{type_}_{seq_type}.pdf')
        plt.figure()


if __name__ == '__main__':
    by_one_seq_graphs()
    simple_graphs('s', True)
    simple_graphs('r')

