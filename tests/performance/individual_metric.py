try:
    import cunumeric as np
except:
    import numpy as np

import time as t

vs = 1000000
mr = 10000
mc = 10000

def vec_add():
    start = t.time()
    v1 = np.random.random((vs))
    v2 = np.random.random((vs))

    v3 = np.add(v1, v2)
    val = np.sum(v3)
    end = t.time()

    print(f"Vector Addition Time: {end - start}")

def vec_sub():
    start = t.time()
    v1 = np.random.random((vs))
    v2 = np.random.random((vs))

    v3 = np.subtract(v1, v2)
    val = np.sum(v3)
    end = t.time()

    print(f"Vector Subtraction Time: {end - start}")

def vec_dot():
    start = t.time()
    v1 = np.random.random((vs))
    v2 = np.random.random((vs))

    res = np.dot(v1, v2)
    end = t.time()

    print(f"Vector Dot Time: {end - start}")

def mat_add():
    start = t.time()
    m1 = np.random.random((mr, mc))
    m2 = np.random.random((mr, mc))

    m3 = np.add(m1, m2)
    end = t.time()

    print(f"Matrix Addition Time: {end - start}")

def mat_sub():
    start = t.time()
    m1 = np.random.random((mr, mc))
    m2 = np.random.random((mr, mc))

    m3 = np.subtract(m1, m2)
    end = t.time()

    print(f"Matrix Subtraction Time: {end - start}")

def mat_vec_dot():
    start = t.time()
    m1 = np.random.random((mr, mc))
    v1 = np.random.random((mc))

    vres = np.dot(m1, v1)
    val = np.sum(vres)
    end = t.time()

    print(f"Matrix-Vector Product Time: {end - start}")

def vec_mat_dot():
    start = t.time()
    m1 = np.random.random((mr, mc))
    v1 = np.random.random((mr))

    vres = np.dot(v1, m1)
    val = np.sum(vres)
    end = t.time()

    print(f"Vector-Matrix Product Time: {end - start}")

vec_add()
vec_sub()
vec_dot()
mat_add()
mat_sub()
mat_vec_dot()
vec_mat_dot()
