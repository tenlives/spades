#include "constructHashTable.hpp"
#include "graphConstruction.hpp"
#include "../sequence.hpp"
#include "common.hpp"



int testFind() {
	Sequence s("ACATACAGACATACA");
	cerr<<s.Str()<<endl;
	Sequence ss = s.Subseq(5,10);
	cerr<<s.Str()<<" "<<ss.Str()<<endl;
	int t = s.find(ss);
	cout << t;
	return 0;
}

int testSimilar() {
	Sequence s("ACATACAGACATACA");
	Sequence t("ATACAGACATACATT");
	forn(i, 4) {
		int ii = i+1;
		//cout << ii;
		int k = s.similar(t, ii);
		cout << k;
		 k = t.similar(s, ii);
		cout << k;
	}
	return 0;
}

int main() {
	//	freopen("error_log", "w",stderr);
	//readsToPairs(parsed_reads, parsed_k_l_mers);
	pairsToSequences(parsed_k_l_mers, parsed_k_sequence);
//	map<>sequencesToMap(parsed_k_sequence);
//	constructGraph();
//	testSimilar();
//	testFind();
	return 0;
}
