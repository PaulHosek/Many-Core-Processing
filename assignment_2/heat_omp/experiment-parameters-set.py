import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import matplotlib as mpl

plt.style.use('seaborn-v0_8-darkgrid')
mpl.rcParams['font.size'] = 16

def plot_hi_lo(ax, x_vals, mean_y, sem, color):
    #upper limit
    ax.plot(x_vals, mean_y + 1.96 * sem, alpha = 0)
    ax.fill_between(x_vals, mean_y, mean_y + 1.96 * sem, alpha = 0.2, color = color)
    #lower limit
    ax.plot(x_vals, mean_y - 1.96 * sem, alpha = 0)
    ax.fill_between(x_vals, mean_y, mean_y - 1.96 * sem, alpha = 0.2, color = color)

def plot_varying_threads(ax, n, m):
    points_to_plot = df[(df['n'] == n and df['m'] == m)]

    xs = np.arange(1,20,1)
    ys = points_to_plot['flops']['mean']

    ax.plot(xs, ys, label="n = " + str(n) + " m = " + str(m), color='#3AD684')
    plot_hi_lo(ax, xs, ys, xs['flops']['sem'], '#3AD684')

def make_plots():
    df = pd.read_csv('data_assignment2')
    df = df.groupby(["m","n"]).agg(['mean','sem']).reset_index()
    print(df)

make_plots()