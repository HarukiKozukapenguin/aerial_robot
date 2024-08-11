#!/usr/bin/env python3
import csv
import matplotlib.pyplot as plt
import argparse

def main(csv_file = "real.csv"):
    body_size = 0.30 #[radius]
    figure, axes = plt.subplots()
    with open(csv_file, 'r') as f:
        reader = csv.reader(f)
        for row in reader:
            x = float(row[1])
            y = float(row[2])
            r = float(row[8])+body_size
            draw_circle = plt.Circle((x, y), r)
            axes.add_artist(draw_circle)
    plt.title('ObstacleMap')
    plt.xlim([-0.2, 3.2])
    plt.ylim([-1.95, 1.95])
    plt.show()
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='drow obstacle circle')
    parser.add_argument('csv_file', type=str, help='CSV file path')
    args = parser.parse_args()
    main(args.csv_file)
