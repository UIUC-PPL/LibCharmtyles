import numpy as np
import time
import sys

dim = 16384
if len(sys.argv) > 1:
    dim = int(sys.argv[1])
print(f'Matrix Dimensions: {dim}x{dim}')
print(f'Vector Dimensions: {dim}')

A = np.full((dim, dim), 1.)
b = np.full((dim), 0.1)
x = np.full((dim), .5)

for i in range(dim):
    for j in range(dim):
        A[i][j] = (i + j) % 10

# b = np.random.random((dim))
# x = np.random.random((dim))

start = time.time()

r = b - np.dot(A, x)
p = r
rsold = np.dot(r, r)
# print(f'[-1] Rsold: {rsold}')

for i in range(100):
    Ap = np.dot(A, p)
    alpha = rsold / np.dot(p, Ap)

    x = x + (alpha * p)
    r = r - (alpha * Ap)

    rsnew = np.dot(r, r)
    if (np.sqrt(rsnew) < 1E-8):
        print(f'Converged in {i} iterations!')
        break

    p = r + (rsnew / rsold) * p
    rsold = rsnew

    # print(f'[{i}] Rsold: {rsold}')

end = time.time()
print(f'Execution Time: {end - start}')
print(f'Rsold: {rsold}')
