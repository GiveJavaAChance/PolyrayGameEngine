package polyray.systems;

public class BVH {

    private final int dim;
    private final Node[] nodes;
    public final float[][] bounds;
    private final int[] indices;
    private int nodeCount = 0;

    public BVH(float[][] bounds, int dimensions) {
        this.dim = dimensions;
        this.bounds = bounds;
        this.indices = new int[bounds.length];
        for (int i = 0; i < indices.length; i++) {
            indices[i] = i;
        }
        this.nodes = new Node[bounds.length << 1];
        buildNode(0, bounds.length);
    }

    private int buildNode(int start, int end) {
        int nodeIndex = nodeCount++;
        nodes[nodeIndex] = new Node();

        if (end - start == 1) {
            float[] b = bounds[start];
            System.arraycopy(b, 0, nodes[nodeIndex].bounds, 0, b.length);
            nodes[nodeIndex].left = -1;
            nodes[nodeIndex].right = -1;
            nodes[nodeIndex].boxIndex = start;
            return nodeIndex;
        }

        float[] bound = nodes[nodeIndex].bounds;
        for (int i = 0; i < dim; i++) {
            bound[i] = Float.POSITIVE_INFINITY;
            bound[i + dim] = Float.NEGATIVE_INFINITY;
        }
        for (int i = start; i < end; i++) {
            float[] b = bounds[i];
            for (int j = 0; j < dim; j++) {
                float min = b[j];
                if (min < bound[j]) {
                    bound[j] = min;
                }
                int idx = j + dim;
                float max = b[idx];
                if (max > bound[idx]) {
                    bound[idx] = max;
                }
            }
        }

        nodes[nodeIndex].boxIndex = -1;

        float[] size = new float[dim << 1];
        for (int i = 0; i < dim; i++) {
            size[i] = bound[i + dim] - bound[i];
        }
        int axis = -1;
        float maxSize = 0.0f;
        for (int i = 0; i < dim; i++) {
            if (size[i] > maxSize) {
                maxSize = size[i];
                axis = i;
            }
        }
        int mid = (start + end) >>> 1;
        quickSelect(bounds, indices, start, end - 1, mid, axis, dim);
        
        if (mid == start) {
            mid++;
        }
        if (mid == end) {
            mid--;
        }

        int left = buildNode(start, mid);
        int right = buildNode(mid, end);
        nodes[nodeIndex].left = left;
        nodes[nodeIndex].right = right;

        return nodeIndex;
    }

    public int query(float[] query, int[] hits) {
        int hitCount = 0;
        int sp = 0;
        int[] stack = new int[nodes.length];
        stack[sp++] = 0;

        while (sp > 0) {
            int nodeIdx = stack[--sp];
            Node node = nodes[nodeIdx];
            boolean overlap = true;
            for (int i = 0; i < dim; i++) {
                if (query[i + dim] < node.bounds[i] || query[i] > node.bounds[i + dim]) {
                    overlap = false;
                    break;
                }
            }
            if (!overlap) {
                continue;
            }
            if (node.boxIndex != -1) {
                hits[hitCount++] = node.boxIndex;
            } else {
                if (node.left != -1) {
                    stack[sp++] = node.left;
                }
                if (node.right != -1) {
                    stack[sp++] = node.right;
                }
            }
        }
        return hitCount;
    }

    public int queryIntersection(float[] pos, float[] dir, float[] dist, RayIntersectionFunction intersectionFunc) {
        int sp = 0;
        int[] stack = new int[nodes.length];
        stack[sp++] = 0;
        int hitIndex = -1;
        float closest = Float.POSITIVE_INFINITY;

        while (sp > 0) {
            int nodeIdx = stack[--sp];
            Node node = nodes[nodeIdx];
            if (!rayIntersectsAABB(pos, dir, node.bounds)) {
                continue;
            }
            if (node.boxIndex != -1) {
                float d = intersectionFunc.hit(node.boxIndex, node.bounds);
                if (d >= 0.0f && d < closest) {
                    closest = d;
                    hitIndex = node.boxIndex;
                }
            } else {
                if (node.left != -1) {
                    stack[sp++] = node.left;
                }
                if (node.right != -1) {
                    stack[sp++] = node.right;
                }
            }
        }
        if (hitIndex != -1) {
            dist[0] = closest;
        }
        return hitIndex;
    }

    private boolean rayIntersectsAABB(float[] pos, float[] dir, float[] bounds) {
        float tMin = Float.NEGATIVE_INFINITY;
        float tMax = Float.POSITIVE_INFINITY;
        for (int i = 0; i < dim; i++) {
            float invD = 1.0f / dir[i];
            float t0 = (bounds[i] - pos[i]) * invD;
            float t1 = (bounds[i + dim] - pos[i]) * invD;
            if (invD < 0) {
                float tmp = t0;
                t0 = t1;
                t1 = tmp;
            }
            if (t0 > tMin) {
                tMin = t0;
            }
            if (t1 < tMax) {
                tMax = t1;
            }
            if (tMax < tMin) {
                return false;
            }
        }
        return tMax >= 0.0f;
    }

    @FunctionalInterface
    public static interface RayIntersectionFunction {

        public float hit(int index, float[] bounds);
    }

    private class Node {

        public Node() {
            this.bounds = new float[dim << 1];
        }

        public final float[] bounds;
        public int left, right;
        public int boxIndex;
    }

    private static void quickSelect(float[][] bounds, int[] indices, int left, int right, int k, int axis, int dim) {
        while (left < right) {
            int pivot = partition(bounds, indices, left, right, axis, dim);
            if (k < pivot) {
                right = pivot - 1;
            } else if (k > pivot) {
                left = pivot + 1;
            } else {
                return;
            }
        }
    }

    private static int partition(float[][] bounds, int[] indices, int low, int high, int axis, int dim) {
        float[] pivot = bounds[indices[high]];
        float pivotVal = (pivot[axis] + pivot[axis + dim]) * 0.5f;
        int i = low - 1;
        for (int j = low; j < high; j++) {
            float[] b = bounds[indices[j]];
            float center = (b[axis] + b[axis + dim]) * 0.5f;
            if (center <= pivotVal) {
                i++;
                swap(indices, i, j);
            }
        }
        swap(indices, i + 1, high);
        return i + 1;
    }

    public static void swap(int[] indices, int a, int b) {
        int tmp = indices[a];
        indices[a] = indices[b];
        indices[b] = tmp;
    }
}
