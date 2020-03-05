/*
 * INPUT:
 *   - clusters must be numbered 1..N.  Furthermore, they
 *     must appear in order and start at 1.
 * KNOBS:
 *   - define DEBUG to cause _alot_ of debugging output.
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <tr1/unordered_map>
#include <vector>
#include <cstdlib>

using namespace std;

#include "ClusterContainer.h"
#include "Readpair.h"
#include "Cluster.h"
#include "ReadInfo.h"

list<ReadInfo*>
select2(ClusterContainer &con,
	tr1::unordered_map<string, ReadInfo> &readinfo,
	Cluster &cl, ostream &ostr, int id, int sid)
{
	list<Readpair> &reads = cl.get_reads();

	/* Build a list of readinfo pointers and print out the reads */
	/* XXX: this read list should really be owned by ClusterContainer */
	/* rather than passed by value back to main() */
	list<ReadInfo*> R;
	for (list<Readpair>::iterator rp = reads.begin();
	     rp != reads.end();
	     ++rp) {
		ReadInfo &ri = readinfo[rp->get_name()];
		if (!ri.is_used())
			R.push_back(&ri);

		ri.set_used();
		ostr << id << "\t" << sid << "\t"
		     << cl.get_id() << "\t" << cl.get_weight() << "\t" << cl.get_mismatches() << "\t"
		     << *rp << "\n";

		/* Mark each cluster containing this read dirty */
		for (ReadInfo::iterator it = ri.begin(); it != ri.end(); ++it)
			con.set_dirty((*it).first);
	}

	return R;
}

int
main(int argc, const char **argv)
{
	long nreads = 0;
	unsigned long cid;
	vector<Cluster*> clusters;
	tr1::unordered_map<string, ReadInfo> readinfo;

	if (argc != 2) {
		cerr << "usage: " << argv[0] << " mapfile" << endl;
		exit(1);
	}

	cout << "Reading input file " << argv[1] << endl;
	ifstream ifs(argv[1]);
	string line;
	while (!getline(ifs, line).eof()) {
		++nreads;
		istringstream linestream(line);
		linestream >> cid;
		if (cid > clusters.size())
			clusters.push_back(new Cluster(cid));

		Readpair rp(linestream);

		/* the memory for each readpair is _owned by the cluster_ */
		ReadInfo::value_type
			elem(clusters[cid - 1], clusters[cid - 1]->add(rp));
		readinfo[rp.get_name()].push_back(elem);
	}

	/* Build the container */
	cout << "Building container [" << clusters.size() << " clusters, "
	     << nreads << " reads ("
	     << readinfo.size() << " unique reads)]"  << endl;
	ClusterContainer con;
	vector<Cluster*>::iterator it = clusters.begin();
	for ( ; it != clusters.end(); ++it) {
#ifdef DEBUG
		cout << "DEBUG: " << *it << "\n";
#endif
		con.add(*it);
	}

	/* Select the best cluster, write it, fix the container */
	cout << "Starting selection process.." << endl;
	int cluster_id = 0;
	while (con.get_size() > 0) {
		int secondary_id = 0;
		list<ReadInfo*> reads;
		list<Cluster*> cls = con.best();
		for (list<Cluster*>::iterator it = cls.begin();
		     it != cls.end();
		     ++it) {
			list<ReadInfo*> new_reads =
				select2(con, readinfo, **it, cout,
			        	cluster_id, secondary_id++);
			reads.splice(reads.begin(), new_reads);
		}
		con.fix(reads);
		++cluster_id;
	}

	return 0;
}
