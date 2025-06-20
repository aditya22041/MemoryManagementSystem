#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import sys

def plot_results(csv_path):
    """Reads benchmark CSV and generates plots."""
    try:
        df = pd.read_csv(csv_path)
    except FileNotFoundError:
        print(f"Error: The file '{csv_path}' was not found.")
        print("Please run the benchmark first using 'make bench'")
        sys.exit(1)

    sns.set_theme(style="whitegrid")
    
    # Plot 1: Average Allocation Time
    plt.figure(figsize=(12, 7))
    ax1 = sns.barplot(
        x="workload",
        y="avg_alloc_time_ns",
        hue="strategy",
        data=df,
        palette="viridis"
    )
    ax1.set_title("Average Allocation Time per Workload", fontsize=16, fontweight='bold')
    ax1.set_xlabel("Workload Type", fontsize=12)
    ax1.set_ylabel("Average Allocation Time (nanoseconds)", fontsize=12)
    ax1.bar_label(ax1.containers[0], fmt='%.0f ns')
    ax1.bar_label(ax1.containers[1], fmt='%.0f ns')
    plt.tight_layout()
    plt.savefig("benchmark_avg_alloc_time.png")
    print("Generated plot: benchmark_avg_alloc_time.png")

    # Plot 2: Final Fragmentation
    plt.figure(figsize=(12, 7))
    ax2 = sns.barplot(
        x="workload",
        y="final_fragmentation",
        hue="strategy",
        data=df,
        palette="plasma"
    )
    ax2.set_title("Memory Fragmentation after Workload", fontsize=16, fontweight='bold')
    ax2.set_xlabel("Workload Type", fontsize=12)
    ax2.set_ylabel("Fragmentation Ratio (0.0 to 1.0)", fontsize=12)
    ax2.bar_label(ax2.containers[0], fmt='%.3f')
    ax2.bar_label(ax2.containers[1], fmt='%.3f')
    plt.tight_layout()
    plt.savefig("benchmark_fragmentation.png")
    print("Generated plot: benchmark_fragmentation.png")

if __name__ == "__main__":
    plot_results("benchmark_results.csv")
