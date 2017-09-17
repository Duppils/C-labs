import java.util.Iterator;
import java.util.ListIterator;
import java.util.LinkedList;
import java.util.ArrayList;
import java.util.BitSet;

class Random {
	int	w;
	int	z;

	public Random(int seed)
	{
		w = seed + 1;
		z = seed * seed + seed + 2;
	}

	int nextInt()
	{
		z = 36969 * (z & 65535) + (z >> 16);
		w = 18000 * (w & 65535) + (w >> 16);

		return (z << 16) + w;
	}
}

class Vertex {
	int			index;
	boolean			listed;
	LinkedList<Vertex>	pred;
	LinkedList<Vertex>	succ;
	BitSet			in;
	BitSet			out;
	BitSet			use;
	BitSet			def;

	int					threadId;

	Vertex(int i)
	{
		index	= i;
		pred	= new LinkedList<Vertex>();
		succ	= new LinkedList<Vertex>();
		in	= new BitSet();
		out	= new BitSet();
		use	= new BitSet();
		def	= new BitSet();
	}

	void computeIn(LinkedList<Vertex> worklist)
	{
		int			i;
		BitSet			old;
		Vertex			v;
		ListIterator<Vertex>	iter;

		iter = succ.listIterator();

		while (iter.hasNext()) {
			v = iter.next();
			out.or(v.in);
		}

		old = in;

		// in = use U (out - def)

		in = new BitSet();
		in.or(out);
		in.andNot(def);
		in.or(use);

		if (!in.equals(old)) {
			iter = pred.listIterator();

			while (iter.hasNext()) {
				v = iter.next();
				if (!v.listed) {
					worklist.addLast(v);
					v.listed = true;
				}
			}
		}
	}

	void parComputeIn(LinkedList<Vertex> worklist, FlowThread[] threadList)
	{
		int			i;
		BitSet			old;
		Vertex			v;
		ListIterator<Vertex>	iter;

		iter = succ.listIterator();

		while (iter.hasNext()) {
			v = iter.next();
			out.or(v.in);
		}

		old = in;

		// in = use U (out - def)

		in = new BitSet();
		in.or(out);
		in.andNot(def);
		in.or(use);

		// if (v.in.threadId != v.out.threadId)

		if (!in.equals(old)) {
			iter = pred.listIterator();

			while (iter.hasNext()) {
				v = iter.next();
				threadList[v.threadId].syncAdd(v);
			}
		}
	}

	public void print()
	{
		int	i;

		System.out.print("use[" + index + "] = { ");
		for (i = 0; i < use.size(); ++i)
			if (use.get(i))
				System.out.print("" + i + " ");
		System.out.println("}");
		System.out.print("def[" + index + "] = { ");
		for (i = 0; i < def.size(); ++i)
			if (def.get(i))
				System.out.print("" + i + " ");
		System.out.println("}\n");

		System.out.print("in[" + index + "] = { ");
		for (i = 0; i < in.size(); ++i)
			if (in.get(i))
				System.out.print("" + i + " ");
		System.out.println("}");

		System.out.print("out[" + index + "] = { ");
		for (i = 0; i < out.size(); ++i)
			if (out.get(i))
				System.out.print("" + i + " ");
		System.out.println("}\n");
	}

	public boolean compare(Vertex other)
	{
		int	i;
		boolean result = true;

		if(!use.equals(other.use)){
			result = false;
			System.out.print("use[" + index + "] = { ");
			for (i = 0; i < use.size(); ++i)
				if (use.get(i))
					System.out.print("" + i + " ");
			System.out.println("}");

			System.out.print("use[" + index + "] = { ");
			for (i = 0; i < other.use.size(); ++i)
				if (other.use.get(i))
					System.out.print("" + i + " ");
			System.out.println("}");
		}

		if(!def.equals(other.def)){
			result = false;
			System.out.print("def[" + index + "] = { ");
			for (i = 0; i < def.size(); ++i)
				if (def.get(i))
					System.out.print("" + i + " ");
			System.out.println("}");

			System.out.print("def[" + index + "] = { ");
			for (i = 0; i < other.def.size(); ++i)
				if (other.def.get(i))
					System.out.print("" + i + " ");
			System.out.println("}");
		}

		if(!in.equals(other.in)){
			result = false;
			System.out.print("in[" + index + "] = { ");
			for (i = 0; i < in.size(); ++i)
				if (in.get(i))
					System.out.print("" + i + " ");
			System.out.println("}");

			System.out.print("in[" + index + "] = { ");
			for (i = 0; i < other.in.size(); ++i)
				if (other.in.get(i))
					System.out.print("" + i + " ");
			System.out.println("}");
		}

		if(!out.equals(other.out)){
			result = false;
			System.out.print("out[" + index + "] = { ");
			for (i = 0; i < out.size(); ++i)
				if (out.get(i))
					System.out.print("" + i + " ");
			System.out.println("}");

			System.out.print("out[" + index + "] = { ");
			for (i = 0; i < other.out.size(); ++i)
				if (other.out.get(i))
					System.out.print("" + i + " ");
			System.out.println("}");
		}
		return result;
	}

}

class Dataflow {

	public static void connect(Vertex pred, Vertex succ)
	{
		pred.succ.addLast(succ);
		succ.pred.addLast(pred);
	}

	public static void generateCFG(Vertex vertex[], int maxsucc, Random r)
	{
		int	i;
		int	j;
		int	k;
		int	s;	// number of successors of a vertex.

		System.out.println("generating CFG...");

		connect(vertex[0], vertex[1]);
		connect(vertex[0], vertex[2]);

		for (i = 2; i < vertex.length; ++i) {
			s = (r.nextInt() % maxsucc) + 1;
			for (j = 0; j < s; ++j) {
				k = Math.abs(r.nextInt()) % vertex.length;
				connect(vertex[i], vertex[k]);
			}
		}
	}

	public static void generateUseDef(
		Vertex	vertex[],
		int	nsym,
		int	nactive,
		Random	r)
	{
		int	i;
		int	j;
		int	sym;

		System.out.println("generating usedefs...");

		for (i = 0; i < vertex.length; ++i) {
			for (j = 0; j < nactive; ++j) {
				sym = Math.abs(r.nextInt()) % nsym;

				if (j % 4 != 0) {
					if (!vertex[i].def.get(sym))
						vertex[i].use.set(sym);
				} else {
					if (!vertex[i].use.get(sym))
						vertex[i].def.set(sym);
				}
			}
		}
	}

	public static double liveness(Vertex vertex[])
	{
		Vertex			u;
		Vertex			v;
		int			i;
		LinkedList<Vertex>	worklist;
		long			begin;
		long			end;

		System.out.println("computing liveness...");

		begin = System.nanoTime();
		worklist = new LinkedList<Vertex>();

		for (i = 0; i < vertex.length; ++i) {
			worklist.addLast(vertex[i]);
			vertex[i].listed = true;
		}

		while (!worklist.isEmpty()) {
			u = worklist.remove();
			u.listed = false;
			u.computeIn(worklist);
		}
		end = System.nanoTime();

		return (end-begin)/1e9;
	}

	public static double parliveness(Vertex vertex[], int threadCount)
	{
		Vertex			u;
		Vertex			v;
		int			i;
		long			begin;
		long			end;
		ArrayList<LinkedList<Vertex>> partitions;
		FlowThread[] threads;


		System.out.println("computing parliveness...");

		begin = System.nanoTime();

		partitions = new ArrayList<>();
		for (i = 0; i < threadCount; i++) {
			partitions.add(new LinkedList<Vertex>());
		}

		for (i = 0; i < vertex.length; ++i) {
			v = vertex[i];
			int index = i % threadCount;
			partitions.get(index).addLast(v); //maybe change way it partitions
			v.listed = true;
			v.threadId = index;
		}

		threads = new FlowThread[threadCount];
		for (i = 0; i < threadCount; i++) {
			threads[i] = new FlowThread(partitions.get(i), i);
		}

		for(FlowThread t : threads) {
			t.addThreadList(threads);
		}

		for (FlowThread t : threads){
			t.start();
		}

		try {
			for (FlowThread t : threads) {
				t.join();
			}
		} catch (InterruptedException e) {
			e.printStackTrace();
		}


		end = System.nanoTime();

		//System.out.println("T = " + (end-begin)/1e9 + " s");
		return (end-begin)/1e9;
	}

	public static void main(String[] args)
	{
		int	i;
		int	nsym;
		int	nvertex;
		int	maxsucc;
		int	nactive;
		int	nthread;
		boolean	print;
		Vertex	vertex[];
		Vertex 	parVertex[];

		nsym = Integer.parseInt(args[0]);
		nvertex = Integer.parseInt(args[1]);
		maxsucc = Integer.parseInt(args[2]);
		nactive = Integer.parseInt(args[3]);
		nthread = Integer.parseInt(args[4]);
		print = Integer.parseInt(args[5]) != 0;

		System.out.println("nsym = " + nsym);
		System.out.println("nvertex = " + nvertex);
		System.out.println("maxsucc = " + maxsucc);
		System.out.println("nactive = " + nactive);

		vertex = new Vertex[nvertex];
		parVertex = new Vertex[nvertex];

		for (i = 0; i < vertex.length; ++i) {
			vertex[i] = new Vertex(i);
			parVertex[i] = new Vertex(i);
		}

		int tries = 1;
		double parSum = 0;
		double seqSum = 0;

		Random parRandom = new Random(1);
		Random seqRandom = new Random(1);

		for (int k = 0; k < tries; k++){
			for (i = 0; i < vertex.length; ++i) {
				parVertex[i] = new Vertex(i);
				vertex[i] = new Vertex(i);
			}

			generateCFG(vertex, maxsucc, seqRandom);
			generateUseDef(vertex, nsym, nactive, seqRandom);
			seqSum += liveness(vertex);

			generateCFG(parVertex, maxsucc, parRandom);
			generateUseDef(parVertex, nsym, nactive, parRandom);
			parSum += parliveness(parVertex, 2);



			for (i = 0; i < vertex.length; ++i){
				if(!vertex[i].compare(parVertex[i])) {
					System.out.println("Wrong result!");
					//break;
				}
			}

		}
		System.out.println("parT = " + parSum/tries + " s");
		System.out.println("seqT = " + seqSum/tries + " s");

		if (print) {
			boolean same = true;
			for (i = 0; i < vertex.length; ++i){
				if(!vertex[i].compare(parVertex[i])) {
					same = false;
				}
			}
			if (same) {
				System.out.println("Correct result!");
			} else {
				System.out.println("Wrong result!");
			}
		}
	}
}

class FlowThread extends Thread {

	private int id;
	private LinkedList<Vertex> worklist;
	private FlowThread[] threadList;

	public FlowThread(LinkedList<Vertex> worklist, int id) {
		this.id = id;
		this.worklist = worklist;
	}

	public void addThreadList(FlowThread[] threadList) {
		this.threadList = threadList;
	}

	public synchronized void syncAdd(Vertex v) {
		if (!v.listed) {
			worklist.addLast(v);
			v.listed = true;
		}
	}

	public synchronized Vertex syncRemove() {
		Vertex u = worklist.remove();
		u.listed = false;
		return u;
	}

	public synchronized boolean syncEmpty() {
		return worklist.isEmpty();
	}

	public void run() {
		while (!syncEmpty()) {
			syncRemove().parComputeIn(worklist, threadList);
		}
	}
}
