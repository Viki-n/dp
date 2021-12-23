import numpy as np

seed = range(10000000).__iter__()

for size in np.logspace(1, 8):
    size = int(size)
    length = max(int(1e7), size*10)
    for type_ in 'srub':
        if type_ in 'ub':
            if size > 1e7:
                continue
            for subset_size in np.logspace(1,8, 8):
                subset_size = int(subset_size)
                if subset_size<size:
                    print(size, length, seed.__next__(), type_, subset_size)
        else:
            print(size, length, seed.__next__(), type_)

for size in range(3,27):
    size = 2**size
    length = int(1e7)
    type_ = 'i'
    print(size, length, seed.__next__(), type_)