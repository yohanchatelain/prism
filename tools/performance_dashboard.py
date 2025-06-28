#!/usr/bin/env python3
"""
Interactive Performance Dashboard for PRISM

This tool generates interactive performance reports using Plotly to visualize
performance evolution over git commits and identify performance regressions.
"""

import json
import os
import glob
import argparse
import subprocess
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple
import pandas as pd
import plotly.graph_objects as go
import plotly.express as px
from plotly.subplots import make_subplots
import plotly.offline as pyo


class PerformanceAnalyzer:
    """Analyzes performance data and generates interactive reports."""
    
    def __init__(self, benchmark_dir: str = "benchmark_results"):
        self.benchmark_dir = Path(benchmark_dir)
        self.data_cache = {}
        
    def load_benchmark_data(self) -> pd.DataFrame:
        """Load all benchmark JSON files from the results directory."""
        json_files = list(self.benchmark_dir.glob("benchmark_*.json"))
        
        if not json_files:
            print(f"No benchmark files found in {self.benchmark_dir}")
            return []
        
        data = pd.DataFrame()
        for json_file in json_files:
            try:
                benchmark_data = pd.read_json(json_file, orient='records')
                data = pd.concat([data, benchmark_data], ignore_index=True)
            except Exception as e:
                print(f"Error loading {json_file}: {e}")
        
        # Sort by timestamp
        data.sort_values(by='timestamp', inplace=True)
        return data
    
    def process_data_for_plotting(self, data: pd.DataFrame) -> pd.DataFrame:
        """Convert benchmark data to pandas DataFrame for easier plotting."""
        rows = []

        commits = data.commit_hash.unique()
        for commit_hash in commits:
            benchmark_run = data[data['commit_hash'] == commit_hash]
            timestamp = benchmark_run.timestamp.unique()[0]
            build_config = benchmark_run.build_config.unique()[0]
            cpu_info = benchmark_run.cpu_info.unique()[0]
            
            # Parse timestamp
            try:
                dt = datetime.strptime(timestamp, '%Y-%m-%d_%H-%M-%S')
            except:
                dt = datetime.now()
            
            for benchmark_name, stats in benchmark_run.get('benchmarks', {}).items():
                row = {
                    'commit_hash': commit_hash,
                    'commit_short': commit_hash[:8] if commit_hash != 'unknown' else 'unknown',
                    'timestamp': timestamp,
                    'datetime': dt,
                    'build_config': build_config,
                    'cpu_info': cpu_info,
                    'benchmark_name': benchmark_name,
                    'operation_name': stats.get('operation_name', ''),
                    'data_type': stats.get('data_type', ''),
                    'vector_size': stats.get('vector_size', 0),
                    'median_time': stats.get('median_time', 0),
                    'mean_time': stats.get('mean_time', 0),
                    'min_time': stats.get('min_time', 0),
                    'max_time': stats.get('max_time', 0),
                    'stddev_time': stats.get('stddev_time', 0),
                    'p95_time': stats.get('p95_time', 0),
                    'p99_time': stats.get('p99_time', 0),
                    'throughput_mops': stats.get('throughput_mops', 0),
                    'iterations': stats.get('iterations', 0),
                    'elements_processed': stats.get('elements_processed', 0),
                }
                rows.append(row)
        
        return pd.DataFrame(rows)
    
    def create_performance_trend_plot(self, df: pd.DataFrame, operation: str) -> go.Figure:
        """Create a performance trend plot for a specific operation across commits."""
        # Filter data for the specific operation
        op_data = df[df['operation_name'] == operation].copy()
        
        if op_data.empty:
            print(f"No data found for operation: {operation}")
            return go.Figure()
        
        # Create subplots for different data types and sizes
        unique_types = op_data['data_type'].unique()
        unique_sizes = sorted(op_data['vector_size'].unique())
        
        fig = make_subplots(
            rows=len(unique_types),
            cols=len(unique_sizes),
            subplot_titles=[f"{dtype} - Size {size}" for dtype in unique_types for size in unique_sizes],
            shared_yaxes=True,
            vertical_spacing=0.1,
            horizontal_spacing=0.05
        )
        
        colors = px.colors.qualitative.Set3
        
        for i, dtype in enumerate(unique_types):
            for j, size in enumerate(unique_sizes):
                subset = op_data[(op_data['data_type'] == dtype) & (op_data['vector_size'] == size)]
                
                if not subset.empty:
                    # Sort by datetime for proper trend line
                    subset = subset.sort_values('datetime')
                    
                    # Main trend line (median time)
                    fig.add_trace(
                        go.Scatter(
                            x=subset['datetime'],
                            y=subset['median_time'],
                            mode='lines+markers',
                            name=f'{dtype} {size} (median)',
                            line=dict(color=colors[j % len(colors)]),
                            customdata=subset[['commit_short', 'throughput_mops']],
                            hovertemplate='<b>%{fullData.name}</b><br>' +
                                        'Time: %{x}<br>' +
                                        'Median Time: %{y:.2f} ns<br>' +
                                        'Commit: %{customdata[0]}<br>' +
                                        'Throughput: %{customdata[1]:.2f} MOPS<br>' +
                                        '<extra></extra>'
                        ),
                        row=i+1, col=j+1
                    )
                    
                    # Error bars (min/max)
                    fig.add_trace(
                        go.Scatter(
                            x=list(subset['datetime']) + list(subset['datetime'][::-1]),
                            y=list(subset['min_time']) + list(subset['max_time'][::-1]),
                            fill='toself',
                            fillcolor=colors[j % len(colors)].replace('rgb', 'rgba').replace(')', ', 0.2)'),
                            line=dict(color='rgba(255,255,255,0)'),
                            name=f'{dtype} {size} (range)',
                            showlegend=False,
                            hoverinfo='skip'
                        ),
                        row=i+1, col=j+1
                    )
        
        fig.update_layout(
            title=f'Performance Trend: {operation} Operation',
            xaxis_title='Time',
            yaxis_title='Execution Time (ns)',
            height=400 * len(unique_types),
            width=300 * len(unique_sizes),
            showlegend=True,
            hovermode='closest'
        )
        
        return fig
    
    def create_throughput_comparison_plot(self, df: pd.DataFrame) -> go.Figure:
        """Create a throughput comparison plot across different operations."""
        # Get latest data for each operation
        latest_data = df.loc[df.groupby(['operation_name', 'data_type', 'vector_size'])['datetime'].idxmax()]
        
        fig = go.Figure()
        
        operations = latest_data['operation_name'].unique()
        colors = px.colors.qualitative.Set1
        
        for i, operation in enumerate(operations):
            op_data = latest_data[latest_data['operation_name'] == operation]
            
            fig.add_trace(go.Bar(
                x=[f"{row['data_type']} {row['vector_size']}" for _, row in op_data.iterrows()],
                y=op_data['throughput_mops'],
                name=operation,
                marker_color=colors[i % len(colors)],
                text=op_data['throughput_mops'].round(2),
                textposition='auto',
                hovertemplate='<b>%{fullData.name}</b><br>' +
                            'Configuration: %{x}<br>' +
                            'Throughput: %{y:.2f} MOPS<br>' +
                            '<extra></extra>'
            ))
        
        fig.update_layout(
            title='Throughput Comparison (Latest Results)',
            xaxis_title='Data Type and Vector Size',
            yaxis_title='Throughput (MOPS)',
            barmode='group',
            height=600,
            xaxis_tickangle=-45
        )
        
        return fig
    
    def create_regression_detection_plot(self, df: pd.DataFrame, threshold: float = 0.1) -> go.Figure:
        """Create a plot highlighting potential performance regressions."""
        # Calculate performance change between consecutive commits
        regressions = []
        
        for operation in df['operation_name'].unique():
            for dtype in df['data_type'].unique():
                for size in df['vector_size'].unique():
                    subset = df[
                        (df['operation_name'] == operation) & 
                        (df['data_type'] == dtype) & 
                        (df['vector_size'] == size)
                    ].sort_values('datetime')
                    
                    if len(subset) < 2:
                        continue
                    
                    for i in range(1, len(subset)):
                        prev_perf = subset.iloc[i-1]['throughput_mops']
                        curr_perf = subset.iloc[i]['throughput_mops']
                        
                        if prev_perf > 0:  # Avoid division by zero
                            change = (curr_perf - prev_perf) / prev_perf
                            
                            if change < -threshold:  # Performance regression
                                regressions.append({
                                    'operation': operation,
                                    'data_type': dtype,
                                    'vector_size': size,
                                    'datetime': subset.iloc[i]['datetime'],
                                    'commit': subset.iloc[i]['commit_short'],
                                    'change_percent': change * 100,
                                    'prev_throughput': prev_perf,
                                    'curr_throughput': curr_perf
                                })
        
        if not regressions:
            return go.Figure().add_annotation(
                text="No performance regressions detected",
                xref="paper", yref="paper",
                x=0.5, y=0.5, showarrow=False
            )
        
        reg_df = pd.DataFrame(regressions)
        
        fig = go.Figure()
        
        operations = reg_df['operation'].unique()
        colors = px.colors.qualitative.Set2
        
        for i, operation in enumerate(operations):
            op_data = reg_df[reg_df['operation'] == operation]
            
            fig.add_trace(go.Scatter(
                x=op_data['datetime'],
                y=op_data['change_percent'],
                mode='markers',
                name=operation,
                marker=dict(
                    color=colors[i % len(colors)],
                    size=10,
                    symbol='x'
                ),
                customdata=op_data[['commit', 'data_type', 'vector_size', 'prev_throughput', 'curr_throughput']],
                hovertemplate='<b>%{fullData.name}</b><br>' +
                            'Time: %{x}<br>' +
                            'Performance Change: %{y:.1f}%<br>' +
                            'Commit: %{customdata[0]}<br>' +
                            'Config: %{customdata[1]} %{customdata[2]}<br>' +
                            'Before: %{customdata[3]:.2f} MOPS<br>' +
                            'After: %{customdata[4]:.2f} MOPS<br>' +
                            '<extra></extra>'
            ))
        
        fig.add_hline(y=-threshold*100, line_dash="dash", line_color="red", 
                     annotation_text=f"Regression Threshold ({threshold*100:.0f}%)")
        
        fig.update_layout(
            title=f'Performance Regressions (>{threshold*100:.0f}% degradation)',
            xaxis_title='Time',
            yaxis_title='Performance Change (%)',
            height=500,
            hovermode='closest'
        )
        
        return fig
    
    def create_size_scaling_plot(self, df: pd.DataFrame, operation: str) -> go.Figure:
        """Create a plot showing how performance scales with vector size."""
        op_data = df[df['operation_name'] == operation].copy()
        
        if op_data.empty:
            return go.Figure()
        
        # Use latest data for each size
        latest_data = op_data.loc[op_data.groupby(['data_type', 'vector_size'])['datetime'].idxmax()]
        
        fig = go.Figure()
        
        data_types = latest_data['data_type'].unique()
        colors = px.colors.qualitative.Pastel
        print(data_types)
        for i, dtype in enumerate(data_types):
            dtype_data = latest_data[latest_data['data_type'] == dtype].sort_values('vector_size')

            fig.add_trace(go.Scatter(
                x=dtype_data['vector_size'],
                y=dtype_data['throughput_mops'],
                mode='lines+markers',
                name=dtype,
                line=dict(color=colors[i % len(colors)]),
                marker=dict(size=8),
                hovertemplate='<b>%{fullData.name}</b><br>' +
                            'Vector Size: %{x}<br>' +
                            'Throughput: %{y:.2f} MOPS<br>' +
                            '<extra></extra>'
            ))
        
        fig.update_layout(
            title=f'Performance Scaling: {operation}',
            xaxis_title='Vector Size',
            yaxis_title='Throughput (MOPS)',
            xaxis_type='log',
            height=500
        )
        
        return fig
    
    def generate_performance_report(self, output_file: str = "performance_report.html"):
        """Generate a comprehensive interactive performance report."""
        data = self.load_benchmark_data()
        
        if data.empty:
            print("No benchmark data available for report generation")
            return
        
        df = self.process_data_for_plotting(data)
        
        print(f"Loaded {len(data)} benchmark runs with {len(df)} data points")
        
        # Create all plots
        operations = df['operation_name'].unique()
        
        # Start building the HTML report
        html_content = """
        <!DOCTYPE html>
        <html>
        <head>
            <title>PRISM Performance Dashboard</title>
            <script src="https://cdn.plot.ly/plotly-latest.min.js"></script>
            <style>
                body { font-family: Arial, sans-serif; margin: 20px; }
                .plot-container { margin: 20px 0; }
                .summary { background-color: #f0f0f0; padding: 15px; border-radius: 5px; margin-bottom: 20px; }
                h1, h2 { color: #333; }
                .metric { display: inline-block; margin: 10px 20px; }
                .metric-value { font-size: 1.5em; font-weight: bold; color: #0066cc; }
                .metric-label { font-size: 0.9em; color: #666; }
            </style>
        </head>
        <body>
            <h1>PRISM Performance Dashboard</h1>
        """
        
        # Add summary statistics
        latest_run = data.sort_values('timestamp').iloc[-1]
        html_content += f"""
            <div class="summary">
                <h2>Latest Benchmark Summary</h2>
                <div class="metric">
                    <div class="metric-value">{latest_run.get('commit_hash', 'unknown')[:8]}</div>
                    <div class="metric-label">Latest Commit</div>
                </div>
                <div class="metric">
                    <div class="metric-value">{latest_run.get('timestamp', 'unknown')}</div>
                    <div class="metric-label">Timestamp</div>
                </div>
                <div class="metric">
                    <div class="metric-value">{latest_run.get('build_config', 'unknown')}</div>
                    <div class="metric-label">Build Config</div>
                </div>
                <div class="metric">
                    <div class="metric-value">{len(operations)}</div>
                    <div class="metric-label">Operations Tested</div>
                </div>
            </div>
        """
        
        # Generate plots and add to HTML
        plot_id = 0
        
        # Throughput comparison
        plot_id += 1
        throughput_fig = self.create_throughput_comparison_plot(df)
        html_content += f'<div class="plot-container"><div id="plot{plot_id}"></div></div>'
        
        # Regression detection
        plot_id += 1
        regression_fig = self.create_regression_detection_plot(df)
        html_content += f'<div class="plot-container"><div id="plot{plot_id}"></div></div>'
        
        # Performance trends for each operation
        trend_plots = []
        for operation in operations:
            plot_id += 1
            trend_fig = self.create_performance_trend_plot(df, operation)
            trend_plots.append(trend_fig)
            html_content += f'<div class="plot-container"><div id="plot{plot_id}"></div></div>'
        
        # Size scaling plots
        scaling_plots = []
        for operation in operations:
            plot_id += 1
            scaling_fig = self.create_size_scaling_plot(df, operation)
            scaling_plots.append(scaling_fig)
            html_content += f'<div class="plot-container"><div id="plot{plot_id}"></div></div>'
        
        # Add JavaScript to render plots
        html_content += """
            <script>
        """
        
        plot_id = 1
        html_content += f"Plotly.newPlot('plot{plot_id}', {throughput_fig.to_json()});\n"
        
        plot_id += 1
        html_content += f"Plotly.newPlot('plot{plot_id}', {regression_fig.to_json()});\n"
        
        for i, trend_fig in enumerate(trend_plots):
            plot_id += 1
            html_content += f"Plotly.newPlot('plot{plot_id}', {trend_fig.to_json()});\n"
        
        for i, scaling_fig in enumerate(scaling_plots):
            plot_id += 1
            html_content += f"Plotly.newPlot('plot{plot_id}', {scaling_fig.to_json()});\n"
        
        html_content += """
            </script>
        </body>
        </html>
        """
        
        # Write the HTML file
        with open(output_file, 'w') as f:
            f.write(html_content)
        
        print(f"Performance report generated: {output_file}")
        return output_file


def main():
    parser = argparse.ArgumentParser(description='Generate PRISM performance dashboard')
    parser.add_argument('--benchmark-dir', default='benchmark_results',
                       help='Directory containing benchmark JSON files')
    parser.add_argument('--output', default='performance_report.html',
                       help='Output HTML file name')
    parser.add_argument('--regression-threshold', type=float, default=0.1,
                       help='Performance regression threshold (default: 10%)')
    
    args = parser.parse_args()
    
    analyzer = PerformanceAnalyzer(args.benchmark_dir)
    analyzer.generate_performance_report(args.output)
    
    print(f"\nTo view the report, open {args.output} in your web browser")


if __name__ == '__main__':
    main()