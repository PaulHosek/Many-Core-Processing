import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import matplotlib as mpl

plt.style.use('seaborn-v0_8-darkgrid')
mpl.rcParams['font.size'] = 16

df = pd.read_csv('data.csv')
maxiter = 2000

#squares, k = 1
squares_k1 = df[df['n'] == df['m'] and df['k'] == 1]
print(squares_k1)

xs_k1 = squares_k1['n']
ys_k1 = squares_k1['flops']

#squares, k = maxiter
squares_kmaxiter = df[df['n'] == df['m'] and df['k'] == maxiter]
print(squares_kmaxiter)

xs_kmaxiter = squares_kmaxiter['n']
ys_kmaxiter = squares_kmaxiter['flops']

#thin rects, k = 1
thin_rects_k1 = df[df['m'] == 100 and df['k'] == 1]
print(thin_rects_k1)

xs_thin_k1 = thin_rects_k1['n']
ys_thin_k1 = thin_rects_k1['flops']

#thin rects, k = 1
thin_rects_kmaxiter = df[df['m'] == 100 and df['k'] == maxiter]
print(thin_rects_kmaxiter)

xs_thin_kmaxiter = thin_rects_kmaxiter['n']
ys_thin_kmaxiter = thin_rects_kmaxiter['flops']

wide_rects_k1 = df[df['n'] == 100 and df['k'] == 1]
print(wide_rects_k1)

xs_wide_k1 = wide_rects_k1['m']
ys_wide_k1 = wide_rects_k1['flops']

wide_rects_kmaxiter = df[df['n'] == 100 and df['k'] == maxiter]
print(wide_rects_kmaxiter)

xs_wide_kmaxiter = wide_rects_kmaxiter['m']
xs_wide_kmaxiter = wide_rects_kmaxiter['flops']
