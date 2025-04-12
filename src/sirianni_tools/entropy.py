"""Calculate entropy-based metrics for track and race analysis."""

import numpy as np
from scipy.stats import entropy
import logging
import blocks

# Configuration for entropy calculations
DEFAULT_BINS = 30
MIN_SAMPLES = 10  # Minimum samples needed for meaningful entropy

def validate_block_data(block_data):
    """Validate block_data structure and required fields."""
    if block_data is None:
        logging.warning("Block data is None - cannot compute entropy metrics")
        return False
    
    # Check first segment has expected structure
    if not block_data or len(block_data[0]) <= max(blocks.SPEED, blocks.RADIUS, 
                                                  blocks.ACCEL, blocks.BRAKE):
        logging.warning(f"Block data segments missing expected fields. Length: {len(block_data[0]) if block_data else 0}")
        return False
    return True

def compute_distribution_entropy(values, bins='auto', name='metric'):
    if not values:
        logging.warning(f"No {name} data available for entropy calculation")
        return None
    
    # Remove NaN/infinite values
    values = np.array(values)
    values = values[np.isfinite(values)]
    
    if len(values) < MIN_SAMPLES:
        logging.warning(f"Insufficient {name} samples ({len(values)} < {MIN_SAMPLES}) for entropy calculation")
        return None
        
    try:
        hist, bin_edges = np.histogram(values, bins=bins, density=False)
        # Convert to probabilities
        probabilities = hist / np.sum(hist) if np.sum(hist) > 0 else hist
        # Filter out zeros to avoid log(0)
        probabilities = probabilities[probabilities > 0]
        # Calculate Shannon entropy
        entropy_value = -np.sum(probabilities * np.log2(probabilities))
        return entropy_value
    except Exception as e:
        logging.error(f"Error computing {name} entropy: {str(e)}")
        return None

def compute_speed_entropy(block_data, bins=DEFAULT_BINS):
    """Calculate entropy of speed distribution across track segments."""
    if not validate_block_data(block_data):
        return None
        
    speeds = []
    for seg in block_data:
        if len(seg) > blocks.SPEED and isinstance(seg[blocks.SPEED], (list, np.ndarray)):
            speeds.extend(seg[blocks.SPEED])
    
    return compute_distribution_entropy(speeds, bins, 'speed')

def compute_curvature_entropy(block_data, bins=DEFAULT_BINS):
    """Calculate entropy of curvature distribution across track segments."""
    if not validate_block_data(block_data):
        return None
        
    curvatures = []
    for seg in block_data:
        if len(seg) > blocks.RADIUS and seg[blocks.RADIUS] != 0:
            # Convert radius to curvature (1/R)
            curvature = 1.0 / float(seg[blocks.RADIUS])
            if np.isfinite(curvature):
                curvatures.append(curvature)
    
    return compute_distribution_entropy(curvatures, bins, 'curvature')

def compute_acceleration_entropy(block_data, bins=DEFAULT_BINS):
    """Calculate entropy of acceleration distribution."""
    if not validate_block_data(block_data):
        return None
        
    accels = []
    for seg in block_data:
        if len(seg) > blocks.ACCEL and isinstance(seg[blocks.ACCEL], (list, np.ndarray)):
            accels.extend(seg[blocks.ACCEL])
    
    return compute_distribution_entropy(accels, bins, 'acceleration')

def compute_braking_entropy(block_data, bins=DEFAULT_BINS):
    """Calculate entropy of braking distribution."""
    if not validate_block_data(block_data):
        return None
        
    braking = []
    for seg in block_data:
        if len(seg) > blocks.BRAKE and isinstance(seg[blocks.BRAKE], (list, np.ndarray)):
            braking.extend(seg[blocks.BRAKE])
    
    return compute_distribution_entropy(braking, bins, 'braking')