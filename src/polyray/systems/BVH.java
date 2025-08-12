package polyray.systems;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;

public class BVH {

    private final int dim;
    private final Node[] nodes;
    public final ArrayList<float[]> bounds;
    public final int[] stack;
    private int nodeCount = 0;

    public BVH(ArrayList<float[]> bounds, int dimensions) {
        this.dim = dimensions;
        this.bounds = bounds;
        this.nodes = new Node[bounds.size() << 1];
        buildNode(0, bounds.size());
        this.stack = new int[nodes.length];
    }

    private int buildNode(int start, int end) {
        int nodeIndex = nodeCount++;
        nodes[nodeIndex] = new Node();

        if (end - start == 1) {
            float[] b = bounds.get(start);
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
            float[] b = this.bounds.get(i);
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
        Comparator<float[]> cmp = Comparator.comparingDouble(b -> (b[selectAxis] + b[selectAxis + dim]) * 0.5f);
        Collections.sort(bounds.subList(start, end), cmp);

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

    public int query(float[] qMin, float[] qMax, int[] hits) {
        int hitCount = 0;
        int sp = 0;
        stack[sp++] = 0;

        while (sp > 0) {
            int nodeIdx = stack[--sp];
            Node node = nodes[nodeIdx];
            boolean overlap = true;
            for (int i = 0; i < dim; i++) {
                if (qMax[i] < node.bounds[i] || qMin[i] > node.bounds[i + dim]) {
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

    private class Node {

        public Node() {
            this.bounds = new float[dim << 1];
        }

        public final float[] bounds;
        public int left, right;
        public int boxIndex;
    }
}
