#pragma once
#include "common/modules/path_extend/pipeline/launch_support.hpp"
#include "common/modules/path_extend/extension_chooser.hpp"
#include "common/modules/path_extend/scaffolder2015/connection_condition2015.hpp"
#include "common/modules/path_extend/scaffolder2015/scaffold_graph.hpp"
#include "common/modules/path_extend/read_cloud_path_extend/transitions/transitions.hpp"
#include "common/modules/path_extend/read_cloud_path_extend/intermediate_scaffolding/gap_closer_predicates.hpp"

namespace path_extend {
    //Same as AssemblyGraphConnectionCondition, but stops after reaching unique edges.
    class AssemblyGraphUniqueConnectionCondition : public AssemblyGraphConnectionCondition {
        using AssemblyGraphConnectionCondition::g_;
        using AssemblyGraphConnectionCondition::interesting_edge_set_;
        using AssemblyGraphConnectionCondition::max_connection_length_;
        //fixme duplication with interesting edges, needed to pass to dijkstra
        const ScaffoldingUniqueEdgeStorage& unique_storage_;
     public:
        AssemblyGraphUniqueConnectionCondition(const Graph& g,
                                               size_t max_connection_length,
                                               const ScaffoldingUniqueEdgeStorage& unique_edges);
        map<EdgeId, double> ConnectedWith(EdgeId e) const override;
        virtual bool IsLast() const override;
    };

    class ScaffoldEdgePredicate {
     public:
        typedef scaffold_graph::ScaffoldGraph ScaffoldGraph;
        typedef scaffold_graph::ScaffoldGraph::ScaffoldEdge ScaffoldEdge;

        virtual bool Check(const scaffold_graph::ScaffoldGraph::ScaffoldEdge& scaffold_edge) const = 0;
        virtual ~ScaffoldEdgePredicate() = default;
    };

    struct ReadCloudMiddleDijkstraParams {
      const double score_threshold_;
      const size_t count_threshold_;
      const size_t middle_count_threshold_;
      const size_t tail_threshold_;
      const size_t len_threshold_;
      const size_t distance_;

      ReadCloudMiddleDijkstraParams(const double score_threshold, const size_t count_threshold_, size_t middle_count_threshold,
                            const size_t tail_threshold_, const size_t len_threshold_,
                            const size_t distance);
    };

    class ReadCloudMiddleDijkstraPredicate: public ScaffoldEdgePredicate {
        using ScaffoldEdgePredicate::ScaffoldEdge;

        const Graph& g;
        const path_extend::ScaffoldingUniqueEdgeStorage& unique_storage_;
        const barcode_index::FrameBarcodeIndexInfoExtractor& barcode_extractor_;
        const ReadCloudMiddleDijkstraParams params_;
     public:
        ReadCloudMiddleDijkstraPredicate(const Graph& g,
                                         const ScaffoldingUniqueEdgeStorage& unique_storage_,
                                         const barcode_index::FrameBarcodeIndexInfoExtractor& barcode_extractor_,
                                         const ReadCloudMiddleDijkstraParams& params);
        bool Check(const ScaffoldEdge& scaffold_edge) const override;

    };

    struct PairedEndDijkstraParams {
      const size_t paired_lib_index_;
      const size_t prefix_length_;
      const config::dataset& dataset_info;
      const path_extend::PathExtendParamsContainer pe_params_;

      PairedEndDijkstraParams(const size_t paired_lib_index_,
                              const size_t prefix_length_,
                              const config::dataset &dataset_info,
                              const PathExtendParamsContainer &pe_params_);
    };

    class PairedEndDijkstraPredicate: public ScaffoldEdgePredicate {
        using ScaffoldEdgePredicate::ScaffoldEdge;

        const Graph& g_;
        const path_extend::ScaffoldingUniqueEdgeStorage& unique_storage_;
        const omnigraph::de::PairedInfoIndicesT<Graph>& clustered_indices_;
        const size_t length_bound_;
        const PairedEndDijkstraParams params_;

     public:
        PairedEndDijkstraPredicate(const Graph &g_,
                                   const ScaffoldingUniqueEdgeStorage &unique_storage_,
                                   const de::PairedInfoIndicesT<debruijn_graph::DeBruijnGraph> &clustered_indices_,
                                   const size_t length_bound_,
                                   const PairedEndDijkstraParams &params_);

        bool Check(const scaffold_graph::ScaffoldGraph::ScaffoldEdge& scaffold_edge) const override;

     private:
        shared_ptr<path_extend::ExtensionChooser> ConstructSimpleExtensionChooser() const;

        DECL_LOGGER("PairedEndDijkstraPredicate");
    };

    class EdgeSplitPredicate: public ScaffoldEdgePredicate {
        using ScaffoldEdgePredicate::ScaffoldEdge;
        typedef barcode_index::BarcodeId BarcodeId;

        const Graph& g_;
        const barcode_index::FrameBarcodeIndexInfoExtractor& barcode_extractor_;
        const size_t count_threshold_;
        const double strictness_;
     public:
        EdgeSplitPredicate(const Graph& g_,
                           const barcode_index::FrameBarcodeIndexInfoExtractor& barcode_extractor_,
                           const size_t count_threshold_,
                           double strictness);

        bool Check(const ScaffoldEdge& scaffold_edge) const override;

     private:
        bool CheckOrderingForThreeSegments(const vector<BarcodeId>& first, const vector<BarcodeId>& second,
                                           const vector<BarcodeId>& third, double strictness) const;

        bool CheckOrderingForFourSegments(const vector<BarcodeId>& first, const vector<BarcodeId>& second,
                                          const vector<BarcodeId>& third, const vector<BarcodeId>& fourth) const;

        DECL_LOGGER("EdgeSplitPredicate");
    };

    class EdgeInTheMiddlePredicate {
     public:
        typedef barcode_index::BarcodeId BarcodeId;

     private:
        const Graph& g_;
        const barcode_index::FrameBarcodeIndexInfoExtractor& barcode_extractor_;
        const size_t count_threshold_;
        const double shared_fraction_threshold_;

     public:
        EdgeInTheMiddlePredicate(const Graph& g_,
                                         const barcode_index::FrameBarcodeIndexInfoExtractor& barcode_extractor_,
                                         size_t count_threshold,
                                         double shared_fraction_threshold);

        bool IsCorrectOrdering(const EdgeId& first, const EdgeId& second, const EdgeId& third);
        DECL_LOGGER("EdgeInTheMiddlePredicate");
    };

    class NextFarEdgesPredicate: public ScaffoldEdgePredicate {
     public:
        using ScaffoldEdgePredicate::ScaffoldEdge;
        typedef scaffold_graph::ScaffoldGraph::ScaffoldVertex ScaffoldVertex;

     private:
        const Graph& g_;
        const barcode_index::FrameBarcodeIndexInfoExtractor& barcode_extractor_;
        const size_t count_threshold_;
        const double shared_fraction_threshold_;
        const std::function<vector<ScaffoldVertex>(ScaffoldVertex)>& candidates_getter_;
     public:
        NextFarEdgesPredicate(const Graph& g_,
                              const barcode_index::FrameBarcodeIndexInfoExtractor& barcode_extractor_,
                              const size_t count_threshold_,
                              const double shared_fraction_threshold_,
                              const std::function<vector<ScaffoldVertex>(ScaffoldVertex)>& candidates_getter_);

        bool Check(const ScaffoldEdge& scaffold_edge) const override;
    };

    class SimpleSearcher {
     public:
        typedef scaffold_graph::ScaffoldGraph ScaffoldGraph;
        typedef ScaffoldGraph::ScaffoldVertex ScaffoldVertex;
     private:
        const ScaffoldGraph& scaff_graph_;
        const Graph& g_;
        size_t distance_threshold_;

        struct VertexWithDistance {
          ScaffoldVertex vertex;
          size_t distance;
          VertexWithDistance(const ScaffoldVertex& vertex, size_t distance);
        };

     public:
        SimpleSearcher(const scaffold_graph::ScaffoldGraph& graph_, const Graph& g, size_t distance_);

        vector<ScaffoldVertex> GetReachableVertices(const ScaffoldVertex& vertex, const ScaffoldGraph::ScaffoldEdge& restricted_edge);

        void ProcessVertex(std::queue<VertexWithDistance>& vertex_queue, const VertexWithDistance& vertex,
                           std::unordered_set<ScaffoldVertex>& visited, const ScaffoldGraph::ScaffoldEdge& restricted_edge);

        bool AreEqual(const ScaffoldGraph::ScaffoldEdge& first, const ScaffoldGraph::ScaffoldEdge& second);

        DECL_LOGGER("SimpleSearcher");
    };

    class TransitiveEdgesPredicate: public ScaffoldEdgePredicate {
     public:
        using ScaffoldEdgePredicate::ScaffoldEdge;
        typedef scaffold_graph::ScaffoldGraph::ScaffoldVertex ScaffoldVertex;

     private:
        const scaffold_graph::ScaffoldGraph scaffold_graph_;
        const Graph& g_;
        size_t distance_threshold_;
     public:
        TransitiveEdgesPredicate(const scaffold_graph::ScaffoldGraph& graph, const Graph& g, size_t distance_threshold);

        bool Check(const ScaffoldEdge& scaffold_edge) const override;

        DECL_LOGGER("TransitiveEdgesPredicate");
    };

    class ScaffoldEdgeScoreFunction {
     protected:
        typedef scaffold_graph::ScaffoldGraph ScaffoldGraph;
        typedef scaffold_graph::ScaffoldGraph::ScaffoldEdge ScaffoldEdge;
     public:
        virtual double GetScore(const scaffold_graph::ScaffoldGraph::ScaffoldEdge& edge) const = 0;
        virtual ~ScaffoldEdgeScoreFunction() = default;
    };

    class AbstractBarcodeScoreFunction: public ScaffoldEdgeScoreFunction {
     protected:
        const Graph& graph_;
        const barcode_index::FrameBarcodeIndexInfoExtractor& barcode_extractor_;

     public:
        AbstractBarcodeScoreFunction(
            const Graph& graph_,
            const barcode_index::FrameBarcodeIndexInfoExtractor& barcode_extractor_);
    };

    class NormalizedBarcodeScoreFunction: public AbstractBarcodeScoreFunction {
        using AbstractBarcodeScoreFunction::barcode_extractor_;
        using AbstractBarcodeScoreFunction::graph_;
        const size_t read_count_threshold_;
        const size_t tail_threshold_;
        const size_t total_barcodes_;

     public:
        NormalizedBarcodeScoreFunction(
            const Graph& graph_,
            const barcode_index::FrameBarcodeIndexInfoExtractor& barcode_extractor_,
            const size_t read_count_threshold_,
            const size_t tail_threshold_);

        virtual double GetScore(const scaffold_graph::ScaffoldGraph::ScaffoldEdge& edge) const override;

        DECL_LOGGER("NormalizedBarcodeScoreFunction");
    };

    class TrivialBarcodeScoreFunction: public AbstractBarcodeScoreFunction {
        using AbstractBarcodeScoreFunction::barcode_extractor_;
        using AbstractBarcodeScoreFunction::graph_;
        const size_t read_count_threshold_;
        const size_t tail_threshold_;

     public:
        TrivialBarcodeScoreFunction(
            const Graph& graph_,
            const barcode_index::FrameBarcodeIndexInfoExtractor& barcode_extractor_,
            const size_t read_count_threshold_,
            const size_t tail_threshold_);

        virtual double GetScore(const scaffold_graph::ScaffoldGraph::ScaffoldEdge& edge) const override;
    };

}
