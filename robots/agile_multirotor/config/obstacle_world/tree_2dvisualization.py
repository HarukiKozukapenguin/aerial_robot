#!/usr/bin/env python3
import csv
import matplotlib.pyplot as plt
import argparse

def main(csv_file="real.csv"):
    body_size = 0.30  # Radius to add to the obstacle radius
    time_interval = 0.25  # Time interval in seconds
    start_time = -2.0
    end_time = 4.0

    figure, axes = plt.subplots()

    # Read CSV file and draw circles
    with open(csv_file, 'r') as f:
        reader = csv.reader(f)
        for row in reader:
            x_initial = float(row[1])
            y_initial = float(row[2])
            vx = float(row[3])
            vy = float(row[4])
            r = float(row[8]) + body_size

            # Plot initial position with alpha=1
            initial_circle = plt.Circle((x_initial, y_initial), r, color='blue', alpha=1.0, edgecolor='black')
            axes.add_artist(initial_circle)

            # Plot positions at each time step with alpha=0.5
            time = start_time
            time = start_time
            while time <= end_time:
                if time != 0:
                    x = x_initial + vx * time
                    y = y_initial + vy * time
                    moving_circle = plt.Circle((x, y), r, edgecolor='black', facecolor='none', alpha=0.5)
                    axes.add_artist(moving_circle)
                time += time_interval

            # Draw arrow for velocity if velocity is not zero
            if vx != 0 or vy != 0:
                arrow_length = 0.5  # Length of the arrow
                axes.arrow(x_initial, y_initial, arrow_length * vx, arrow_length * vy,
                           head_width=0.3, head_length=0.3, fc='red', ec='red', linewidth=3)


    # Add red and green crosses only
    axes.plot(0.0, 0.0, 'r+', markersize=15, markeredgewidth=5, label='Start')  # Red cross
    axes.plot(3.0, 0.0, 'g+', markersize=15, markeredgewidth=5, label='Goal')  # Green cross

    # Set title, labels, and axis limits
    plt.xlim([-0.2, 3.2])
    plt.ylim([-1.95, 1.95])
    plt.legend()  # Show legend

    plt.grid(True)
    plt.gca().set_aspect('equal', adjustable='box')  # Ensure equal aspect ratio
    plt.grid(False)  # Disable grid lines
    plt.show()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Draw dynamic obstacle circles and cross markers.')
    parser.add_argument('csv_file', type=str, help='Path to the CSV file')
    args = parser.parse_args()
    main(args.csv_file)
