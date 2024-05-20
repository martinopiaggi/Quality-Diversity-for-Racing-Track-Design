function splineSmoothing(spline) {
    for (let i = 0; i < 10; i++) {
        spline = generateCatmullRomSpline(spline, 5, i * 10);
        spline = pushApart(spline, 20);
        spline = fixAngles(spline);
    }
    spline = generateCatmullRomSpline(spline, 10, 0);
    spline = pushApart(spline, 1);
    return spline;
}

function findMinCurvatureSegment(track, segmentLength) {
    let minAverageCurvature = Infinity;
    let minSegmentStartIndex = 0;
    const trackLength = track.length;

    for (let index = 0; index < trackLength; index++) {
        let totalCurvature = 0;

        for (let offset = 0; offset < segmentLength; offset++) {
            const curvatureIndex = (index + offset) % trackLength;
            const curvature = calculateCurvature(track, curvatureIndex);
            totalCurvature += curvature;
        }

        const averageCurvature = totalCurvature / segmentLength;
        if (averageCurvature < minAverageCurvature) {
            minAverageCurvature = averageCurvature;
            minSegmentStartIndex = index;
        }
    }

    return (minSegmentStartIndex + segmentLength/2) % trackLength;
}

function findMaxCurveBeforeStraight(track, segmentLength) {
    let bestScore = -Infinity;
    let desiredSegmentStartIndex = 0;
    const trackLength = track.length;
    const halfSegmentLength = Math.floor(segmentLength / 2);

    for (let index = 0; index < trackLength; index++) {
        let firstHalfTotalCurvature = 0;
        let secondHalfTotalCurvature = 0;

        // Calculate total curvature for the first half of the segment
        for (let offset = 0; offset < halfSegmentLength; offset++) {
            const curvatureIndex = (index + offset) % trackLength;
            const curvature = calculateCurvature(track, curvatureIndex);
            firstHalfTotalCurvature += curvature;
        }

        // Calculate total curvature for the second half of the segment
        for (let offset = halfSegmentLength; offset < segmentLength; offset++) {
            const curvatureIndex = (index + offset) % trackLength;
            const curvature = calculateCurvature(track, curvatureIndex);
            secondHalfTotalCurvature += curvature;
        }

        const firstHalfAverageCurvature = firstHalfTotalCurvature / halfSegmentLength;
        const secondHalfAverageCurvature = secondHalfTotalCurvature / halfSegmentLength;

        // Score calculation
        const curvatureScore = firstHalfAverageCurvature; // Higher score for higher curvature
        const straightnessScore = 1 / (secondHalfAverageCurvature + 1); // Higher score for lower curvature (more straight)
        
        const totalScore = straightnessScore + curvatureScore;

        // Update if current segment has a better score
        if (totalScore > bestScore) {
            bestScore = totalScore;
            desiredSegmentStartIndex = index;
        }
    }

    return (desiredSegmentStartIndex + segmentLength/2) % trackLength;
}



function calculateCurvature(track, i) {
    const current = track[i];
    const next = track[(i + 1) % track.length];
    const nextNext = track[(i + 2) % track.length];

    const xp = (nextNext.x - current.x) / 2;
    const yp = (nextNext.y - current.y) / 2;
    const xpp = (nextNext.x - 2 * next.x + current.x);
    const ypp = (nextNext.y - 2 * next.y + current.y);

    const numerator = xp * ypp - yp * xpp;
    const denominator = Math.pow((xp * xp + yp * yp), 1.5);

    return denominator !== 0 ? Math.abs(numerator / denominator) : 0;
}

function calculateSegment(point1, point2) {
    const dx = point2.x - point1.x;
    const dy = point2.y - point1.y;
    return Math.sqrt(dx * dx + dy * dy);
}

function calculateCurve(p1, p2, p3) {
    function determinant(x1, y1, x2, y2, x3, y3) {
        return x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2);
    }

    const D = 2 * determinant(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
    if (D === 0) return null;

    const ux = ((p1.x ** 2 + p1.y ** 2) * (p2.y - p3.y) +
                (p2.x ** 2 + p2.y ** 2) * (p3.y - p1.y) +
                (p3.x ** 2 + p3.y ** 2) * (p1.y - p2.y)) / D;
    const uy = ((p1.x ** 2 + p1.y ** 2) * (p3.x - p2.x) +
                (p2.x ** 2 + p2.y ** 2) * (p1.x - p3.x) +
                (p3.x ** 2 + p3.y ** 2) * (p2.x - p1.x)) / D;
    const radius = Math.sqrt((p1.x - ux) ** 2 + (p1.y - uy) ** 2);

    const angle1 = Math.atan2(p1.y - uy, p1.x - ux);
    const angle3 = Math.atan2(p3.y - uy, p3.x - ux);

    let theta = Math.abs(angle3 - angle1) * (180 / Math.PI);
    if (theta > 180) theta = 360 - theta;

    const dir = (p2.x - p1.x) * (p3.y - p2.y) - (p2.y - p1.y) * (p3.x - p2.x) > 0 ? 'rgt' : 'lft';

    return { dir, radius, angle: theta };
}

function pushApart(points, minDistance = 5) {
    const minDistanceSquared = minDistance * minDistance;
    let i = 0;
    while (i < points.length) {
        let removed = false;
        for (let j = i + 1; j < points.length; j++) {
            const dx = points[j].x - points[i].x;
            const dy = points[j].y - points[i].y;
            const distanceSquared = dx * dx + dy * dy;
            if (distanceSquared < minDistanceSquared) {
                points.splice(j, 1);
                removed = true;
                break;
            }
        }
        if (!removed) {
            i++;
        }
    }
    return points;
}

function fixAngles(points) {
    const radDeg = 180 / Math.PI;
    const degRad = Math.PI / 180;
    const maxAngle = 80;

    for (let i = 0; i < points.length; ++i) {
        const previous = (i - 1 < 0) ? points.length - 1 : i - 1;
        const next = (i + 1) % points.length;

        let px = points[i].x - points[previous].x;
        let py = points[i].y - points[previous].y;
        const pl = Math.sqrt(px * px + py * py);
        px /= pl;
        py /= pl;

        let nx = points[next].x - points[i].x;
        let ny = points[next].y - points[i].y;
        const nl = Math.sqrt(nx * nx + ny * ny);
        nx /= nl;
        ny /= nl;

        let a = Math.atan2(px * ny - py * nx, px * nx + py * ny);
        if (Math.abs(a * radDeg) <= maxAngle) continue;

        const nA = maxAngle * Math.sign(a) * degRad;
        const diff = nA - a;
        const cos = Math.cos(diff);
        const sin = Math.sin(diff);

        const newX = nx * cos - ny * sin;
        const newY = nx * sin + ny * cos;
        points[next].x = points[i].x + newX * nl;
        points[next].y = points[i].y + newY * nl;
    }

    return points;
}

function generateCatmullRomSpline(data, steps, startIndex) {
    let spline = [];

    for (let i = startIndex; i < startIndex + data.length; i++) {
        const index = i % data.length;
        const p0 = data[(index + data.length - 1) % data.length];
        const p1 = data[index];
        const p2 = data[(index + 1) % data.length];
        const p3 = data[(index + 2) % data.length];

        for (let t = 0; t <= 1; t += 1 / steps) {
            const t2 = t * t;
            const t3 = t2 * t;

            const b1 = 0.5 * (-t3 + 2 * t2 - t);
            const b2 = 0.5 * (3 * t3 - 5 * t2 + 2);
            const b3 = 0.5 * (-3 * t3 + 4 * t2 + t);
            const b4 = 0.5 * (t3 - t2);

            const x = p0.x * b1 + p1.x * b2 + p2.x * b3 + p3.x * b4;
            const y = p0.y * b1 + p1.y * b2 + p2.y * b3 + p3.y * b4;

            spline.push({ x, y });
        }
    }

    return spline;
}

module.exports = {
    splineSmoothing,
    findMinCurvatureSegment,
    findMaxCurveBeforeStraight,
    calculateCurvature,
    calculateSegment,
    calculateCurve,
    pushApart,
    fixAngles,
    generateCatmullRomSpline
};
