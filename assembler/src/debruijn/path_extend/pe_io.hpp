//***************************************************************************
//* Copyright (c) 2011-2013 Saint-Petersburg Academic University
//* All Rights Reserved
//* See file LICENSE for details.
//****************************************************************************

/*
 * pe_io.hpp
 *
 *  Created on: Mar 15, 2012
 *      Author: andrey
 */

#ifndef PE_IO_HPP_
#define PE_IO_HPP_


#include "bidirectional_path.hpp"

namespace path_extend {

using namespace debruijn_graph;

class ContigWriter {

protected:
    conj_graph_pack& gp_;

    size_t k_;
	string ToString(const BidirectionalPath& path) const {
		stringstream ss;
		if (!path.Empty()) {
			ss << gp_.mismatch_masker.MaskedEdgeNucls(path[0], 0.001).substr(0, k_);
		}

		for (size_t i = 0; i < path.Size(); ++i) {
			int gap = i == 0 ? 0 : path.GapAt(i);
			if (gap > (int) k_) {
				for (size_t j = 0; j < gap - k_; ++j) {
					ss << "N";
				}
				ss << gp_.mismatch_masker.MaskedEdgeNucls(path[i], 0.001);
			} else {
				int overlapLen = k_ - gap;
				if (overlapLen >= (int) gp_.g.length(path[i]) + (int) k_) {
					continue;
				}

				ss << gp_.mismatch_masker.MaskedEdgeNucls(path[i], 0.001).substr(overlapLen);
			}
		}
		return ss.str();
	}

    Sequence ToSequence(const BidirectionalPath& path) const {
        SequenceBuilder result;

        if (!path.Empty()) {
            result.append(gp_.g.EdgeNucls(path[0]).Subseq(0, k_));
        }
        for (size_t i = 0; i < path.Size(); ++i) {
            result.append(gp_.g.EdgeNucls(path[i]).Subseq(k_));
        }

        return result.BuildSequence();
    }



public:
    ContigWriter(conj_graph_pack& gp, size_t k): gp_(gp), k_(k){

    }

    void writeEdges(const string& filename) {
        INFO("Outputting edges to " << filename);
        osequencestream_with_data_for_scaffold oss(filename);

        set<EdgeId> included;
        for (auto iter = gp_.g.SmartEdgeBegin(); !iter.IsEnd(); ++iter) {
            if (included.count(*iter) == 0) {
                oss.setCoverage(gp_.g.coverage(*iter));
                oss.setID(gp_.g.int_id(*iter));
                oss << gp_.g.EdgeNucls(*iter);

                included.insert(*iter);
                included.insert(gp_.g.conjugate(*iter));
            }
        }
        INFO("Contigs written");
    }


    void writePathEdges(PathContainer& paths, const string& filename){
		INFO("Outputting path data to " << filename);
		ofstream oss;
        oss.open(filename.c_str());
		for (size_t i = 0; i < paths.size(); ++i) {
            oss << i << endl;
            BidirectionalPath path = *paths.Get(i);
            oss << "PATH " << paths.Get(i)->GetId() << " " << path.Size() << " " << path.Length() + k_ << endl;
            for (size_t j = 0; j < path.Size(); ++j) {
			    oss << gp_.g.int_id(path[j]) << " " << gp_.g.length(path[j]) << endl;
            }
            oss << endl;
		}
		oss.close();
		INFO("Edges written");
	}

    void writePaths(PathContainer& paths, const string& filename) {

        INFO("Writing contigs to " << filename);
        osequencestream_with_data_for_scaffold oss(filename);
        int i = 0;
        for (auto iter = paths.begin(); iter != paths.end(); ++iter) {
        	if (iter.get()->Length() < k_){
        		continue;
        	}
        	INFO("NODE " << ++i);
        	iter.get()->Print();
        	oss.setID(iter.get()->GetId());
            oss.setCoverage(iter.get()->Coverage());
            oss << ToString(*iter.get());
        }
        INFO("Contigs written");
    }


};


class PathInfoWriter {


public:

    void writePaths(PathContainer& paths, const string& filename){
        ofstream oss(filename);

        for (auto iter = paths.begin(); iter != paths.end(); ++iter) {
            iter.get()->Print(oss);
        }

        oss.close();
    }
};

}

#endif /* PE_IO_HPP_ */
