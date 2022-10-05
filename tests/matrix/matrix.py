import numpy as np
import time

start = time.time()

X = np.full((8192, 8192), 1.)
y = np.full((8192), 2.)

x_dot_y = np.dot(X, y)

end = time.time()

print(f'Execution Time: {end - start}')
