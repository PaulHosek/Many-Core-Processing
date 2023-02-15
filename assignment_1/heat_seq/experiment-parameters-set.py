import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import matplotlib as mpl

plt.style.use('seaborn-v0_8-darkgrid')
mpl.rcParams['font.size'] = 16

pd.set_option('display.max_columns', 500)
df = pd.read_csv('good_data.csv')
df = df.groupby(["m","n","k"]).agg(['mean','sem']).reset_index()


maxiter = 3000

#squares, k = 1
squares_k1 = df[(df['n'] == df['m']) & (df['k'] == 1)]

print(squares_k1)

xs_k1 = squares_k1['n']
ys_k1 = squares_k1['flops']["mean"]
#squares, k = maxiter
def plot_hi_lo(ax, mean_y,x_vals, sem,color):
    #upper limit
    ax.plot(x_vals, mean_y+1.96*sem, alpha=0)
    ax.fill_between(x_vals,mean_y, mean_y+1.96*sem, alpha=0.2, color=color)
    #lower limit
    ax.plot(x_vals, mean_y-1.96*sem, alpha=0)
    ax.fill_between(x_vals,mean_y, mean_y-1.96*sem, alpha=0.2, color=color)

squares_kmaxiter = df[(df['n'] == df['m']) & (df['k'] == maxiter)]
xs_kmaxiter = squares_kmaxiter['n']
ys_kmaxiter = squares_kmaxiter['flops']["mean"]

f, ax = plt.subplots()
ax.set_title("Square grid")
ax.plot(xs_k1, ys_k1, label="k = 1", color='#3AD684')
plot_hi_lo(ax, ys_k1, xs_k1,squares_k1['flops']['sem'],'#3AD684')

ax.plot(xs_kmaxiter, ys_kmaxiter, label="k = maxiter",color='#517075')
plot_hi_lo(ax, ys_kmaxiter, xs_kmaxiter,squares_kmaxiter['flops']['sem'],'#517075')


ax.set(xlabel="Grid size", ylabel="Flops/s")
ax.legend()
# f.show()
f.savefig("squares_test.png", dpi=300, bbox_inches='tight')
raise KeyboardInterrupt
#thin rects, k = 1
thin_rects_k1 = df[(df['m'] == 100) & (df['k'] == 1)]
xs_thin_k1 = thin_rects_k1['n']
ys_thin_k1 = thin_rects_k1['flops']['mean']

#thin rects, k = maxiter
thin_rects_kmaxiter = df[(df['m'] == 100) & (df['k'] == maxiter)]
xs_thin_kmaxiter = thin_rects_kmaxiter['n']
ys_thin_kmaxiter = thin_rects_kmaxiter['flops']['mean']
print(thin_rects_kmaxiter)

f, ax = plt.subplots()
ax.set_title("Thin rectangles")
ax.plot(xs_thin_k1, ys_thin_k1, label="k = 1", color='#3AD684')
ax.plot(xs_thin_kmaxiter, ys_thin_kmaxiter, label="k = maxiter",color='#517075')

plot_hi_lo(ax, ys_thin_k1, xs_thin_k1,thin_rects_kmaxiter['flops']['sem'],'#3AD684')
plot_hi_lo(ax, ys_thin_kmaxiter, xs_thin_kmaxiter, thin_rects_kmaxiter['flops']['sem'],'#517075')

ax.set(xlabel="Grid size", ylabel="Flops/s")
ax.legend()
f.savefig("thin_rectangles.png", dpi=300, bbox_inches='tight')

#wide rects, k = 1
wide_rects_k1 = df[(df['n'] == 100) & (df['k'] == 1)]
xs_wide_k1 = wide_rects_k1['m']
ys_wide_k1 = wide_rects_k1['flops']

#wide rects, k = maxiter
wide_rects_kmaxiter = df[(df['n'] == 100) & (df['k'] == maxiter)]
xs_wide_kmaxiter = wide_rects_kmaxiter['m']
ys_wide_kmaxiter = wide_rects_kmaxiter['flops']

f, ax = plt.subplots()
ax.set_title("Wide rectangles")
ax.plot(xs_wide_k1, ys_wide_k1, label="k = 1", color='#3AD684')
ax.plot(xs_wide_kmaxiter, ys_wide_kmaxiter, label="k = maxiter",color='#517075')
ax.set(xlabel="Grid size", ylabel="Flops/s")
ax.legend()
f.savefig("wide_rectangles.png", dpi=300, bbox_inches='tight')