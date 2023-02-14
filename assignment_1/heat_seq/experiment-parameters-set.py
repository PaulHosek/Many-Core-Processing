import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import matplotlib as mpl

plt.style.use('seaborn-v0_8-darkgrid')
mpl.rcParams['font.size'] = 16

df = pd.read_csv('data.csv')
maxiter = 3000

#squares, k = 1
squares_k1 = df[(df['n'] == df['m']) & (df['k'] == 1)]
xs_k1 = squares_k1['n']
ys_k1 = squares_k1['flops']

#squares, k = maxiter
squares_kmaxiter = df[(df['n'] == df['m']) & (df['k'] == maxiter)]
xs_kmaxiter = squares_kmaxiter['n']
ys_kmaxiter = squares_kmaxiter['flops']

f, ax = plt.subplots()
ax.plot(xs_k1, ys_k1, label="k = 1")
ax.plot(xs_kmaxiter, ys_kmaxiter, label="k = maxiter")
ax.set(xlabel="Grid size", ylabel="Flops/s")
ax.legend()
f.savefig("squares.svg", dpi=300, bbox_inches='tight')

#thin rects, k = 1
thin_rects_k1 = df[(df['m'] == 100) & (df['k'] == 1)]
xs_thin_k1 = thin_rects_k1['n']
ys_thin_k1 = thin_rects_k1['flops']

#thin rects, k = maxiter
thin_rects_kmaxiter = df[(df['m'] == 100) & (df['k'] == maxiter)]
xs_thin_kmaxiter = thin_rects_kmaxiter['n']
ys_thin_kmaxiter = thin_rects_kmaxiter['flops']
print(thin_rects_kmaxiter)

f, ax = plt.subplots()
ax.plot(xs_thin_k1, ys_thin_k1, label="k = 1")
ax.plot(xs_thin_kmaxiter, ys_thin_kmaxiter, label="k = maxiter")
ax.set(xlabel="Grid size", ylabel="Flops/s")
ax.legend()
f.savefig("thin_rectangles.svg", dpi=300, bbox_inches='tight')

#wide rects, k = 1
wide_rects_k1 = df[(df['n'] == 100) & (df['k'] == 1)]
xs_wide_k1 = wide_rects_k1['m']
ys_wide_k1 = wide_rects_k1['flops']

#wide rects, k = maxiter
wide_rects_kmaxiter = df[(df['n'] == 100) & (df['k'] == maxiter)]
xs_wide_kmaxiter = wide_rects_kmaxiter['m']
ys_wide_kmaxiter = wide_rects_kmaxiter['flops']

f, ax = plt.subplots()
ax.plot(xs_wide_k1, ys_wide_k1, label="k = 1")
ax.plot(xs_wide_kmaxiter, ys_wide_kmaxiter, label="k = maxiter")
ax.set(xlabel="Grid size", ylabel="Flops/s")
ax.legend()
f.savefig("wide_rectangles.svg", dpi=300, bbox_inches='tight')