import numpy as np

SEED = 42

for size in np.logspace(1, 8):
    size = int(size)
    length = max(int(1e7), size*10)
    for type_ in 'srub':
        if type_ in 'ub':
            for subset_size in np.logspace(1,8, 8):
                subset_size = int(subset_size)
                if subset_size<size:
                    print(size, length, SEED, type_, subset_size)
        else:
            print(size, length, SEED, type_)