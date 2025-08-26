package polyray.systems;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map.Entry;

public class GLGarbageCollector {

    private static final HashMap<Long, GCQueue> gcQueues = new HashMap<>();
    private static final int MAX_LIFETIME = 2;

    public static void submit(Runnable r) {
        long ID = Thread.currentThread().getId();
        GCQueue queue = gcQueues.get(ID);
        if(queue == null) {
            queue = new GCQueue();
            synchronized(gcQueues) {
                gcQueues.put(ID, queue);
            }
        }
        queue.lifetime = MAX_LIFETIME;
        queue.queue.add(r);
    }
    
    public static void gc() {
        ArrayList<Runnable> queue = new ArrayList<>();
        synchronized(gcQueues) {
            Iterator<Entry<Long, GCQueue>> iter = gcQueues.entrySet().iterator();
            while (iter.hasNext()) {
                Entry<Long, GCQueue> e = iter.next();
                GCQueue q = e.getValue();
                queue.addAll(q.queue);
                q.queue.clear();
                if(q.lifetime <= 0) {
                    iter.remove();
                }
            }
        }
        for (Runnable r : queue) {
            r.run();
        }
    }
    
    private static class GCQueue {

        public final ArrayList<Runnable> queue;
        public int lifetime;

        public GCQueue() {
            this.queue = new ArrayList<>();
            this.lifetime = MAX_LIFETIME;
        }
    }
}
