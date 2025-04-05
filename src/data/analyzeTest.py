import os
import json
import math

JSON_DIR = './tests'

def collect_data():
    try:
        files = os.listdir(JSON_DIR)
        json_files = [f for f in files if f.endswith('.json')]

        # Initialize accumulators for all metrics
        metrics = {
            'length': {'total': 0, 'sum_squares': 0},
            'deltaX': {'total': 0, 'sum_squares': 0},
            'deltaY': {'total': 0, 'sum_squares': 0},
            'deltaAngleDegrees': {'total': 0, 'sum_squares': 0},
            'speed_entropy': {'total': 0, 'sum_squares': 0},
            'acceleration_entropy': {'total': 0, 'sum_squares': 0},
            'braking_entropy': {'total': 0, 'sum_squares': 0},
            'positions_mean': {'total': 0, 'sum_squares': 0},
            'avg_radius_mean': {'total': 0, 'sum_squares': 0},
            'gaps_mean': {'total': 0, 'sum_squares': 0},
            'right_bends': {'total': 0, 'sum_squares': 0},
            'avg_radius_var': {'total': 0, 'sum_squares': 0},
            'total_overtakes': {'total': 0, 'sum_squares': 0},
            'straight_sections': {'total': 0, 'sum_squares': 0},
            'gaps_var': {'total': 0, 'sum_squares': 0},
            'left_bends': {'total': 0, 'sum_squares': 0},
            'positions_var': {'total': 0, 'sum_squares': 0},
            'curvature_entropy': {'total': 0, 'sum_squares': 0}
        }

        count = 0
        perfect_track_count = 0
        very_bad_track_count = 0
        perfect_tracks = []
        very_bad_tracks = []

        # First pass: collect sums for averages
        for file in json_files:
            file_path = os.path.join(JSON_DIR, file)
            with open(file_path, 'r', encoding='utf-8') as f:
                data = json.load(f)
            
            # Check if data is valid and has a non-None 'fitness'
            fitness = data.get('fitness')
            if fitness is None:
                continue

            count += 1

            # Update all metrics
            for key in metrics.keys():
                if key in fitness and fitness[key] is not None:
                    metrics[key]['total'] += fitness[key]

            # Track quality checks
            if (abs(fitness.get('deltaX', 0)) < 1 and
                abs(fitness.get('deltaY', 0)) < 1 and
                abs(fitness.get('deltaAngleDegrees', 0)) < 10):
                perfect_track_count += 1
                perfect_tracks.append({
                    'seed': data.get('id'),
                    'trackSize': data.get('trackSize'),
                    'mode': data.get('mode'),
                    'metrics': fitness.copy()
                })

            if (abs(fitness.get('deltaX', 0)) + abs(fitness.get('deltaY', 0)) > 6):
                very_bad_track_count += 1
                very_bad_tracks.append({
                    'seed': data.get('id'),
                    'trackSize': data.get('trackSize'),
                    'mode': data.get('mode'),
                    'metrics': fitness.copy()
                })

        # Calculate averages
        averages = {}
        for key, value in metrics.items():
            averages[key] = value['total'] / count if count else 0

        # Second pass: calculate variances
        for file in json_files:
            file_path = os.path.join(JSON_DIR, file)
            with open(file_path, 'r', encoding='utf-8') as f:
                data = json.load(f)
            
            # Check again if data is valid and has a non-None 'fitness'
            fitness = data.get('fitness')
            if fitness is None:
                continue

            for key in metrics.keys():
                if key in fitness and fitness[key] is not None:
                    metrics[key]['sum_squares'] += (fitness[key] - averages[key]) ** 2

        # Calculate standard deviations
        std_devs = {}
        for key, value in metrics.items():
            std_devs[key] = math.sqrt(value['sum_squares'] / (count - 1)) if count > 1 else 0

        # Print results
        print(f"\nAnalysis Results ({count} valid tracks):")
        print("\nBasic Statistics:")
        for key in averages:
            print(f"{key}:")
            print(f"  Average: {averages[key]:.4f}")
            print(f"  Std Dev: {std_devs[key]:.4f}")

        print("\nTrack Quality Analysis:")
        print(f"Perfect Tracks: {perfect_track_count}")
        print(f"Problematic Tracks: {very_bad_track_count}")

        if perfect_track_count > 0:
            print("\nPerfect Tracks Details:")
            for track in perfect_tracks:
                print(f"\nSeed: {track['seed']}")
                print(f"Track Size: {track['trackSize']}")
                print(f"Mode: {track['mode']}")
                print("Key Metrics:")
                print(f"  Length: {track['metrics'].get('length', 0):.2f}")
                print(f"  Speed Entropy: {track['metrics'].get('speed_entropy', 0):.2f}")
                print(f"  Curvature Entropy: {track['metrics'].get('curvature_entropy', 0):.2f}")

        if very_bad_track_count > 0:
            print("\nProblematic Tracks Details:")
            for track in very_bad_tracks:
                print(f"\nSeed: {track['seed']}")
                print(f"Track Size: {track['trackSize']}")
                print(f"Mode: {track['mode']}")
                print("Issues:")
                print(f"  Delta X: {track['metrics'].get('deltaX', 0):.2f}")
                print(f"  Delta Y: {track['metrics'].get('deltaY', 0):.2f}")
                print(f"  Delta Angle: {track['metrics'].get('deltaAngleDegrees', 0):.2f}")

    except Exception as e:
        print(f"Error: {e}")

if __name__ == '__main__':
    collect_data()
