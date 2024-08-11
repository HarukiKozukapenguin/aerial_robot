#!/usr/bin/env python3
import csv
import matplotlib.pyplot as plt
import argparse

def main(csv_file="real.csv"):
    body_size = 0.30  # Radius to add to the obstacle radius
    figure, axes = plt.subplots()

    # Read CSV file and draw circles
    with open(csv_file, 'r') as f:
        reader = csv.reader(f)
        for row in reader:
            x = float(row[1])
            y = float(row[2])
            r = float(row[8]) + body_size
            draw_circle = plt.Circle((x, y), r, color='blue', alpha=0.5)
            axes.add_artist(draw_circle)

    # Add red and green crosses only
    axes.plot(0.0, 0.0, 'r+', markersize=15, markeredgewidth=5, label='Start')  # Red cross
    axes.plot(3.0, 0.0, 'g+', markersize=15, markeredgewidth=5, label='Goal')  # Green cross

    # Set title, labels, and axis limits
    # plt.title('Obstacle Map')
    plt.xlim([-0.2, 3.2])
    plt.ylim([-1.95, 1.95])
    # plt.xlabel('x (m)')
    # plt.ylabel('y (m)')
    plt.legend()  # Show legend

    plt.grid(True)
    plt.gca().set_aspect('equal', adjustable='box')  # Ensure equal aspect ratio
    plt.grid(False)  # Disable grid lines
    plt.show()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Draw obstacle circles and cross markers.')
    parser.add_argument('csv_file', type=str, help='Path to the CSV file')
    args = parser.parse_args()
    main(args.csv_file)
