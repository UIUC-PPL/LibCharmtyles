import numpy as np
import time
import sys

dim = 16384
if len(sys.argv) > 1:
    dim = int(sys.argv[1])
print(f'Matrix Dimensions: {dim}x{dim}')
print(f'Vector Dimensions: {dim}')


A = np.full((dim, dim), 1)
b = np.full((dim), 2)

start = time.time()
res = np.dot(A, b)
end = time.time()

print(f'Execution Time: {end - start}')
