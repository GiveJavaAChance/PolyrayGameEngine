package polyray;

import java.util.ArrayList;
import java.util.BitSet;
import java.util.HashMap;
import java.util.HashSet;
import static org.lwjgl.opengl.GL46.*;
import polyray.systems.IDGenerator;

public class ShaderBufferHeap {

    public final ShaderBuffer buffer;
    private final BitSet memoryMask;
    private final int memorySize;
    private final int unitSize;

    private final IDGenerator gen;
    private final HashMap<Integer, Long> blocks;
    private final HashSet<Integer> active;
    private final HashSet<Integer> updated;

    public ShaderBufferHeap(int length, int unitSize) {
        this.buffer = new ShaderBuffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW);
        this.buffer.setSize((length * unitSize) << 2);
        this.memoryMask = new BitSet(length);
        this.memorySize = length;
        this.unitSize = unitSize;
        this.gen = new IDGenerator();
        this.blocks = new HashMap<>();
        this.active = new HashSet<>();
        this.updated = new HashSet<>();
    }

    public final int reserve(int units) {
        int ptr = locate(units);
        int ID = gen.getNewID();
        blocks.put(ID, (long) ptr << 32l | (long) units);
        return ID;
    }

    public final int allocate(float[] data) {
        int size = data.length / unitSize;
        int ptr = locate(size);
        int ID = gen.getNewID();
        blocks.put(ID, (long) ptr << 32l | (long) size);
        buffer.uploadPartialData(data, ptr * unitSize);
        active.add(ID);
        updated.add(ID);
        return ID;
    }

    public final void reallocate(int ID, float[] data) {
        Long m = blocks.get(ID);
        if (m == null) {
            throw new IllegalArgumentException("ID doesn't exist!");
        }
        int newSize = data.length / unitSize;
        int ptr = (int) (m >>> 32l);
        int size = (int) (m & 0xFFFFFFFF);
        if (newSize == size) {
            buffer.uploadPartialData(data, ptr * unitSize);
            updated.add(ID);
            return;
        }
        if (newSize < size) {
            memoryMask.clear(ptr + newSize, ptr + size);
            blocks.put(ID, (long) ptr << 32l | (long) newSize);
            buffer.uploadPartialData(data, ptr * unitSize);
            updated.add(ID);
            return;
        }
        int endIdx = memoryMask.nextSetBit(ptr + size);
        if (endIdx == -1 || endIdx > memorySize) {
            endIdx = memorySize;
        }
        if (endIdx - ptr >= newSize) {
            memoryMask.set(ptr, ptr + newSize);
            blocks.put(ID, (long) ptr << 32l | (long) newSize);
            buffer.uploadPartialData(data, ptr * unitSize);
            updated.add(ID);
            return;
        }
        int newPtr = locate(newSize);
        memoryMask.clear(ptr, ptr + size);
        blocks.put(ID, (long) newPtr << 32l | (long) newSize);
        buffer.uploadPartialData(data, newPtr * unitSize);
        updated.add(ID);
    }

    public final int allocate(int[] data) {
        int size = data.length / unitSize;
        int ptr = locate(size);
        int ID = gen.getNewID();
        blocks.put(ID, (long) ptr << 32l | (long) size);
        buffer.uploadPartialData(data, ptr * unitSize);
        active.add(ID);
        updated.add(ID);
        return ID;
    }

    public final void reallocate(int ID, int[] data) {
        Long m = blocks.get(ID);
        if (m == null) {
            throw new IllegalArgumentException("ID doesn't exist!");
        }
        int newSize = data.length / unitSize;
        int ptr = (int) (m >>> 32l);
        int size = (int) (m & 0xFFFFFFFF);
        if (newSize == size) {
            buffer.uploadPartialData(data, ptr * unitSize);
            updated.add(ID);
            return;
        }
        if (newSize < size) {
            memoryMask.clear(ptr + newSize, ptr + size);
            blocks.put(ID, (long) ptr << 32l | (long) newSize);
            buffer.uploadPartialData(data, ptr * unitSize);
            updated.add(ID);
            return;
        }
        int endIdx = memoryMask.nextSetBit(ptr + size);
        if (endIdx == -1 || endIdx > memorySize) {
            endIdx = memorySize;
        }
        if (endIdx - ptr >= newSize) {
            memoryMask.set(ptr, ptr + newSize);
            blocks.put(ID, (long) ptr << 32l | (long) newSize);
            buffer.uploadPartialData(data, ptr * unitSize);
            updated.add(ID);
            return;
        }
        int newPtr = locate(newSize);
        memoryMask.clear(ptr, ptr + size);
        blocks.put(ID, (long) newPtr << 32l | (long) newSize);
        buffer.uploadPartialData(data, newPtr * unitSize);
        updated.add(ID);
    }

    public final void free(int ID) {
        Long m = blocks.remove(ID);
        if (m == null) {
            return;
        }
        gen.freeID(ID);
        int ptr = (int) (m >>> 32l);
        int size = (int) (m & 0xFFFFFFFF);
        memoryMask.clear(ptr, ptr + size);
        active.remove(ID);
        updated.remove(ID);
    }

    public final int locate(int size) {
        int currentIdx = 0;
        while (currentIdx < memorySize) {
            int startIdx = memoryMask.nextClearBit(currentIdx);
            if (startIdx >= memorySize) {
                break;
            }
            int endIdx = memoryMask.nextSetBit(startIdx);
            if (endIdx == -1 || endIdx > memorySize) {
                endIdx = memorySize;
            }
            if ((endIdx - startIdx) >= size) {
                memoryMask.set(startIdx, startIdx + size);
                return startIdx;
            }
            currentIdx = endIdx + 1;
        }
        throw new RuntimeException("Out of memory!");
    }

    public final void activate(int ID) {
        if (active.add(ID)) {
            updated.add(ID);
        }
    }

    public final void deactivate(int ID) {
        if (active.remove(ID)) {
            updated.add(ID);
        }
    }

    public final int getTotalCount() {
        return this.blocks.size();
    }

    public final int getActiveCount() {
        return this.active.size();
    }

    public final int[] pollUpdates() {
        int[] out = new int[updated.size()];
        int idx = 0;
        for (int i : updated) {
            out[idx++] = i;
        }
        updated.clear();
        return out;
    }

    public final HashMap<Integer, Long> getAllocations() {
        return this.blocks;
    }

    public final HashSet<Integer> getActive() {
        return this.active;
    }

    public final ArrayList<Long> getChunks() {
        BitSet mask = new BitSet(memorySize);
        for (int ID : active) {
            long m = blocks.get(ID);
            int ptr = (int) (m >>> 32l);
            int size = (int) (m & 0xFFFFFFFF);
            mask.set(ptr, ptr + size);
        }
        ArrayList<Long> chunks = new ArrayList<>();
        int idx = 0;
        while (true) {
            int ptr = mask.nextSetBit(idx);
            if (ptr < 0) {
                break;
            }
            int end = mask.nextClearBit(ptr);
            if (end < 0) {
                end = memorySize;
            }
            int size = end - ptr;
            long packed = (long) ptr << 32l | (long) size;
            chunks.add(packed);
            idx = end;
        }
        return chunks;
    }
}
