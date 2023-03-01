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
    ax2.semilogy(xs, ys, label="n = " + str(n) + " m = " + str(m))
    ax2.legend(prop={'size': 6})
    ax2.set(xlabel = "Threads", ylabel = "Time")

def plot_speed_up(ax1, ax2, n, m, df, df_seq, experiment_tuples):
    mean_for_seq = []
    mean_for_one_threads = []
    sem_for_one_threads = []

    mean_for_two_threads = []
    sem_for_two_threads = []

    mean_for_three_threads = []
    sem_for_three_threads = []
    strings = np.array([])
    df_1 = df[df['threads'] == 1]
    df_2 = df[df['threads'] == 2]
    df_3 = df[df['threads'] == 3]
    print(df_1)
    print(df_2)
    print(df_3)
    for experiment_tuple in experiment_tuples:
        strings = np.append(strings, str(experiment_tuple))

        (m, n) = experiment_tuple
        mean_seq = df_seq[(df_seq['n'] == n) & (df_seq['m'] == m)]["time"]["mean"].iloc[0]
        mean_one_threads = df[(df['n'] == n) & (df['m'] == m) & (df['threads'] == 1)]["time"]["mean"].iloc[0]
        mean_two_threads = df[(df['n'] == n) & (df['m'] == m) & (df['threads'] == 2)]["time"]["mean"].iloc[0]
        mean_three_threads = df[(df['n'] == n) & (df['m'] == m) & (df['threads'] == 3)]["time"]["mean"].iloc[0]

        mean_for_seq.append(mean_seq)
        mean_for_one_threads.append(mean_one_threads)
        mean_for_two_threads.append(mean_two_threads)
        mean_for_three_threads.append(mean_three_threads)

    for i in range(0, len(experiment_tuples)):
        firsts = np.ones(5)
        seconds = [i / j for i, j in zip(mean_for_one_threads, mean_for_two_threads)]
        thirds = [i / j for i, j in zip(mean_for_one_threads, mean_for_three_threads)]

        firsts_seq = np.ones(5)
        seconds_seq = [i / j for i, j in zip(mean_for_seq, mean_for_two_threads)]
        thirds_seq = [i / j for i, j in zip(mean_for_seq, mean_for_three_threads)]

    x = np.arange(5)
    width = 0.25  # the width of the bars

    offset = 0
    ax1.bar(x + offset, firsts, width=width, label="1 thread", color="red")
    ax2.bar(x + offset, firsts_seq, width=width, label="sequential", color="red")

    offset+=width
    ax1.bar(x + offset, seconds, width=width, label="2 threads", color="green")
    ax2.bar(x + offset, seconds_seq, width=width, label="2 threads", color="green")

    offset+=width
    ax1.bar(x + offset, thirds, width=width, label="3 threads", color="blue")
    ax2.bar(x + offset, thirds_seq, width=width, label="3 threads", color="blue")

    ax1.tick_params(labelsize=10)
    ax1.set_xticks(x + width, strings)
    ax1.set_xlabel("(m,n)", fontsize=14)
    ax1.set_ylabel("Speed-up", fontsize=14)
    ax1.legend()

    ax2.tick_params(labelsize=10)
    ax2.set_xticks(x + width, strings)
    ax2.set_xlabel("(m,n)", fontsize=14)
    ax2.set_ylabel("Speed-up", fontsize=14)
    ax2.legend(prop={'size': 12})

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
    df_seq = pd.read_csv('data_assignment2_seq.csv')
    df_seq = df_seq.groupby(['n', 'm']).agg(['mean','sem']).reset_index()

    experiment_tuples = [(100, 100), (1000,1000), (100, 20000), (20000, 100), (5000, 5000)]

    fig1,ax1 = plt.subplots()
    fig2,ax2 = plt.subplots()
    fig3,ax3 = plt.subplots()
    fig4,ax4 = plt.subplots()

    for tuple in experiment_tuples:
        (m, n) = tuple
        plot_varying_threads(ax1, ax2, n, m, df)

    plot_speed_up(ax3, ax4, n, m, df, df_seq, experiment_tuples)

    fig1.savefig("flops.png")
    fig2.savefig("times.png")
    fig3.savefig("speed-up_heat.png")
    fig4.savefig("speed-up_heat_seq.png")

make_plots()