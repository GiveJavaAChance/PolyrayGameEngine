package polyray.systems;

import java.util.Comparator;

public class BVH {

    private final int dim;
    private final Node[] nodes;
    public final float[][] bounds;
    public final int[] indices;
    public final int[] stack;
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
        this.stack = new int[nodes.length];
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
        final int selectAxis = axis;
        quickSort(bounds, indices, start, end - 1, Comparator.comparingDouble(b -> (b[selectAxis] + b[selectAxis + dim]) * 0.5f));

        int mid = (start + end) >>> 1;
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
            if(t0 > tMin) {
                tMin = t0;
            }
            if(t1 < tMax) {
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

    private static void quickSort(float[][] bounds, int[] indices, int low, int high, Comparator<float[]> c) {
        if (low < high) {
            int pivotIndex = partition(bounds, indices, low, high, c);
            quickSort(bounds, indices, low, pivotIndex - 1, c);
            quickSort(bounds, indices, pivotIndex + 1, high, c);
        }
    }

    private static int partition(float[][] bounds, int[] indices, int low, int high, Comparator<float[]> c) {
        float[] pivot = bounds[high];
        int i = low - 1;

        for (int j = low; j < high; j++) {
            if (c.compare(bounds[j], pivot) <= 0) {
                i++;
                swap(bounds, indices, i, j);
            }
        }
        swap(bounds, indices, i + 1, high);
        return i + 1;
    }

    public static void swap(float[][] bounds, int[] indices, int a, int b) {
        float[] tmp1 = bounds[a];
        bounds[a] = bounds[b];
        bounds[b] = tmp1;

        int tmp2 = indices[a];
        indices[a] = indices[b];
        indices[b] = tmp2;
    }
}
