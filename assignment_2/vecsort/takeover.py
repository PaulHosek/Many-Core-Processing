import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import matplotlib as mpl
import seaborn as sns

pd.set_option('display.max_columns', None)
plt.style.use('seaborn-v0_8-darkgrid')
mpl.rcParams['font.size'] = 16

# load data
df = pd.read_csv("data/takeover.csv")

# convert to speedup
seq_mean = df[df["version"] == "sequential"]["runtime"].mean()
df["speedup"] = seq_mean / df["runtime"]

df = df.drop(df[df['version'] == 'sequential'].index)


# first add speedup to sequential + sem

# rename and order bars
df = df.replace({"parallel_v1_onlynested": "task",
                 "parallel_v2_onlyouter": "data"})
df["input_size"] = df.inner_max * df.len_outer
df["ratio"] = df.inner_max / df.len_outer
df = df.groupby(["version", "ratio"]).mean()
df.reset_index(inplace=True)

df.columns = ['version', 'ratio', 'inner_max', 'len_outer', 'runtime', 'speedup', 'input_size']
print(df)

# Create a grouped bar chart
plt.figure(figsize=(9,9))

g = sns.scatterplot(x='ratio', y='speedup', hue='version', data=df, palette=["#809F92", "#446469"], legend=False)
sns.lineplot(x='ratio', y='speedup',hue="version", data=df, palette=["#809F92", "#446469"])
# g.legend(loc='center left', bbox_to_anchor=(1.02, 0.5))
legend = g.legend(loc="upper center")
legend.get_frame().set_facecolor('white')
legend.get_frame().set_edgecolor('black')
legend.set_frame_on(True)
# Set the axis labels and title
g.set(xscale='log')
# g.set_axis_labels('len_outer (log scale)', 'runtime (seconds)')
g.set_title('Task vs. Data Parallelism.')
plt.xlabel("Ratio (inner length/ outer length)")
plt.ylabel("Speedup (to sequential)")
# Show the plot
plt.ylim(0)
plt.savefig("takeover_pic.png", bbox_inches="tight", dpi=300)


raise KeyboardInterrupt
# Plot average runtime over len_outer on the first axis
fig, (ax1, ax2) = plt.subplots(ncols=2, figsize=(12, 5))
sns.lineplot(x='inner_max', y='runtime', hue='version', errorbar='sd', ax=ax1, data=df.groupby(['version', 'inner_max']).mean().reset_index())
ax1.set(xscale='log', xlabel='inner_max (log scale)', ylabel='average runtime (seconds)', title='Average runtime over len_outer')

# Plot average runtime over inner_max on the second axis
sns.lineplot(x='len_outer', y='runtime', hue='version', errorbar='sd', ax=ax2, data=df.groupby(['version', 'len_outer']).mean().reset_index())
ax2.set(xscale='log', xlabel='len_outer (log scale)', ylabel='average runtime (seconds)', title='Average runtime over inner_max')

# Move the legend outside the plots
# ax1.legend(loc='center left', bbox_to_anchor=(1.02, 0.5))
ax2.legend(loc='center left', bbox_to_anchor=(1.02, 0.5))

# Set the overall title for the figure
fig.suptitle('Comparison of different versions', fontsize=14)

plt.savefig("takeover_pic.png", bbox_inches="tight", dpi=300)



raise KeyboardInterrupt
# convert to speedup
seq_mean = df[df["version"] == "sequential"]["runtime"].mean()
df["speedup"] = seq_mean / df["runtime"]

# aggregate data for bar plot
df = df.groupby("version").agg(["mean", "sem"])
print(df)
df.reset_index(inplace=True)
df.columns = ['Implementation', 'runtime_mean', 'runtime_sem', 'speedup_mean', 'speedup_sem']

# rename and order bars
df = df.replace({"parallel_v1_onlynested": "task",
                 "parallel_v2_onlyouter": "data",
                 "parallel_v3_both": "both",
                 "parallel_v4_both_o3": "both+O3", })
category_order = ['sequential', 'task', 'data', 'both', 'both+O3']
df['Implementation'] = pd.Categorical(df['Implementation'], categories=category_order, ordered=True)
df = df.sort_values('Implementation')

print(df)
versions = df["Implementation"]
means = df['speedup_mean'].values
sems = df['speedup_sem'].values

fig, ax = plt.subplots(figsize=(9, 9))
ax.bar(versions, means, yerr=1.96 * sems, capsize=5, color="#8FA993")
ax.set_xticks(versions, labels=versions)
ax.set_title("Vecsort speedup comparison", fontsize=16)
ax.set_xlabel("Version", fontsize=14)
ax.set_ylabel("Speedup (to sequential)", fontsize=14)
plt.ylim(0)
plt.xticks(rotation=0)

plt.savefig("vecsort_rel.png", dpi=300, bbox_inches="tight")

# g = sns.catplot(x='inner_max', y='runtime', hue='version', col='len_outer', data=df, kind='violin', height=4, aspect=1)
#
# # Set the axis labels and title
# g.set_axis_labels('inner_max', 'runtime (seconds)')
# g.fig.suptitle('Comparison of different versions')