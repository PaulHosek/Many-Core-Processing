import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import matplotlib as mpl
import os

plt.style.use('seaborn-v0_8-darkgrid')
mpl.rcParams['font.size'] = 16

def plot_varying_threads(ax1, ax2, n, m, df):
    points_to_plot = df[(df['n'] == n) & (df['m'] == m)]
    points_to_plot = points_to_plot.sort_values('threads')

    xs = points_to_plot['threads']

    #flops graph
    ys = points_to_plot['flops']['mean']
    ax1.plot(xs, ys, label="n = " + str(n) + " m = " + str(m))
    ax1.legend(prop={'size': 6})
    ax1.set(xlabel = "Threads", ylabel = "Flops")

    #time graph
    ys = points_to_plot['time']['mean']
    ax2.semilogy(xs, ys, label="n = " + str(n) + " m = " + str(m))
    ax2.legend(prop={'size': 6})
    ax2.set(xlabel = "Threads", ylabel = "Time")

def fix_df(path):
    df = pd.read_csv(path)
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

def compare_seq(ax, df_new_seq, df_old_seq, experiment_tuples):
    seq_new_results = []
    seq_old_results = []
    strings = []

    for experiment_tuple in experiment_tuples:
        strings = np.append(strings, str(experiment_tuple))
        (n,m) = experiment_tuple
        new_result = df_new_seq[(df_new_seq['n'] == n) & (df_new_seq['m'] == m) & (df_new_seq['threads'] == 1)].iloc[0]["flops"]["mean"]
        old_result = df_old_seq[(df_old_seq['n'] == n) & (df_old_seq['m'] == m) & (df_old_seq['threads'] == 1)].iloc[0]["flops"]["mean"]

        seq_new_results.append(new_result)
        seq_old_results.append(old_result)

    x = np.arange(5)
    offset = 0
    width = 0.25  # the width of the bars
    speed_up_new = [i / j for i, j in zip(seq_new_results, seq_old_results)]
    speed_up_old = np.ones(len(speed_up_new))

    ax.bar(x + offset, speed_up_new, width=width, label="new")
    ax.bar(x + width, speed_up_old, width=width, label="old")

    ax.tick_params(labelsize=10)
    ax.set_xticks(x + width, strings)
    ax.set_xlabel("(m,n)", fontsize=14)
    ax.set_ylabel("Flops/s", fontsize=14)
    ax.legend(loc='upper center', bbox_to_anchor=(0.5, -0.05),
          fancybox=True, shadow=True, ncol=5)
    
def compare_versions(ax, ax_speed_up, dict_version_df, experiment_tuples):
    seq_new_results = []
    (fixed_n, fixed_m) = (1000, 1000)
    versions = ["heat_omp", "heat_pth", "heat_pth_v1", "heat_pth_v2", "heat_pth_v3", "heat_pth_v4", "heat_pth_simd"]
    df_new_seq = dict_version_df["heat_seq_new"]

    for experiment_tuple in experiment_tuples:
        (n,m) = experiment_tuple
        new_result = df_new_seq[(df_new_seq['n'] == n) & (df_new_seq['m'] == m) & (df_new_seq['threads'] == 1)].iloc[0]["flops"]["mean"]
        seq_new_results.append(new_result)

    results = np.zeros((7, 3))
    threads = [1, 7, 16]

    for i in range(0, len(versions)):
        (n,m) = (fixed_n, fixed_m)
        for j in range(0, len(threads)):
            result = dict_version_df[versions[i]][(dict_version_df[versions[i]]['n'] == n) & (dict_version_df[versions[i]]['m'] == m) & (dict_version_df[versions[i]]['threads'] == threads[j])].iloc[0]["flops"]["mean"]
            results[i][j] = result
    
    x = np.arange(3)    
    width = 0.12
    offset = 0

    for i in range(0, len(versions)):
        row_to_plot = results[i,:]
        ax.bar(x + offset, row_to_plot, width=width, label=versions[i])
        offset += width

    ax.tick_params(labelsize=10)
    ax.set_xticks(x + width, threads)
    ax.set_xlabel("Threads", fontsize=14)
    ax.set_ylabel("Flops/s", fontsize=14)
    ax.legend()

    x = np.arange(3)    
    width = 0.25
    offset = 0
    
    simd_results = results[(len(versions) - 1), :]

    speed_up_seq = np.ones(3)
    speed_up_simd = [i / j for i, j in zip(simd_results, seq_new_results)]

    ax_speed_up.bar(x + offset, speed_up_seq, width = width, label = "Sequential")
    offset += width
    ax_speed_up.bar(x + offset, speed_up_simd, width = width, label = "Best parallel")

    ax_speed_up.tick_params(labelsize=10)
    ax_speed_up.set_xticks(x + width, threads)
    ax_speed_up.set_xlabel("threads", fontsize=14)
    ax_speed_up.set_ylabel("speed-up", fontsize=14)
    ax_speed_up.legend()

def make_plots():
    experiment_tuples = [(100, 100), (1000,1000), (100, 20000), (20000, 100), (5000, 5000)]
    dirs = ["heat_omp", "heat_pth", "heat_pth_v1", "heat_pth_v2", "heat_pth_v3", "heat_pth_v4", "heat_seq_new", "heat_seq_old", "heat_pth_simd"]
    data_files = ["data_omp.csv", "data_pth_v0.csv", "data_pth_v1.csv", "data_pth_v2.csv", "data_pth_v3.csv", "data_pth_v4.csv", "data_seq_new.csv", "data_seq_old.csv", "data_pth_simd.csv"]
    os.chdir("..")
    dict_version_df = {}

    for i in range(0, len(dirs)):
        os.chdir(dirs[i])
        df = fix_df(data_files[i])
        dict_version_df[dirs[i]] = df
        os.chdir("..")

    fig1, ax1 = plt.subplots()
    compare_seq(ax1, dict_version_df["heat_seq_new"], dict_version_df["heat_seq_old"], experiment_tuples)
    fig1.savefig("seq_comparison")

    fig2, ax2 = plt.subplots()
    fig3, ax3 = plt.subplots()
    compare_versions(ax2, ax3, dict_version_df, experiment_tuples)
    fig2.savefig("version_comparison")
    fig3.savefig("speed-up")

make_plots()