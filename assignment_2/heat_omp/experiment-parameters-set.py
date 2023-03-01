import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import matplotlib as mpl

plt.style.use('seaborn-v0_8-darkgrid')
mpl.rcParams['font.size'] = 16

def plot_hi_lo(ax, x_vals, mean_y, sem, color='#809F82'):
    #upper limit
    ax.plot(x_vals, mean_y + 1.96 * sem, alpha = 0)
    ax.fill_between(x_vals, mean_y, mean_y + 1.96 * sem, alpha = 0.2, color = color)
    #lower limit
    ax.plot(x_vals, mean_y - 1.96 * sem, alpha = 0)
    ax.fill_between(x_vals, mean_y, mean_y - 1.96 * sem, alpha = 0.2, color = color)

def plot_varying_threads(ax1, ax2, n, m, df):
    points_to_plot = df[(df['n'] == n) & (df['m'] == m)]
    points_to_plot = points_to_plot.sort_values('threads')

    xs = points_to_plot['threads']

    #flops graph
    ys = points_to_plot['flops']['mean']
    ax1.plot(xs, ys, label="n = " + str(n) + " m = " + str(m))
    plot_hi_lo(ax1, xs, ys, points_to_plot['flops']['sem'])
    ax1.legend(prop={'size': 6})
    ax1.set(xlabel = "Threads", ylabel = "Flops")

    #time graph
    ys = points_to_plot['time']['mean']
    ax2.plot(xs, ys, label="n = " + str(n) + " m = " + str(m))
    plot_hi_lo(ax2, xs, ys, points_to_plot['time']['sem'])
    ax2.legend(prop={'size': 6})
    ax2.set(xlabel = "Threads", ylabel = "Time")

def fix_df():
    df = pd.read_csv('data_assignment2.csv')
    df1 = pd.DataFrame()
    df1[['a','b','c','d']] = df.threads.str.split(" ", expand=True)
    df1 = df1.drop(columns=['a', 'c'])
    df['threads'] = df1['b']
    df['flops'] = df['time']
    df['time'] = df1['d']
    df['threads'] = pd.to_numeric(df['threads'])
    df['flops'] = pd.to_numeric(df['flops'])
    df['time'] = pd.to_numeric(df['time'])
    df = df.groupby(["n","m","threads"]).agg({'time':['mean','sem'], 'flops':['mean', 'sem']}).reset_index()

    return df

def make_plots():
    df = fix_df()
    experiment_tuples = [(100, 100), (1000,1000), (100, 20000), (20000, 100), (5000, 5000)]

    fig1,ax1 = plt.subplots()
    fig2,ax2 = plt.subplots()

    for tuple in experiment_tuples:
        (m, n) = tuple
        plot_varying_threads(ax1, ax2, n, m, df)

    fig1.savefig("flops.png")
    fig2.savefig("times.png")

make_plots()