#include <iostream>
#include <cstdlib>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/cycle_canceling.hpp>
#include <boost/graph/push_relabel_max_flow.hpp>
#include <boost/graph/successive_shortest_path_nonnegative_weights.hpp>
#include <boost/graph/find_flow_cost.hpp>

/* Solution worth 40p.
 * Basically we just create bipartite graph with cap 1 and cost = -coeff, connect boats to src and owners to sink
 * and run flow. However, we need to add missing edges with cost 0 and cap 1 i.e. there need to be edge from every boat to every owner. 
 * */

typedef boost::adjacency_list_traits<boost::vecS, boost::vecS, boost::directedS> Traits;
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, boost::no_property,
    boost::property<boost::edge_capacity_t, long,
        boost::property<boost::edge_residual_capacity_t, long,
            boost::property<boost::edge_reverse_t, Traits::edge_descriptor,
                boost::property <boost::edge_weight_t, long> > > > > Graph; // new!

typedef boost::property_map<Graph, boost::edge_capacity_t>::type      EdgeCapacityMap;
typedef boost::property_map<Graph, boost::edge_weight_t >::type       EdgeWeightMap; // new!
typedef boost::property_map<Graph, boost::edge_residual_capacity_t>::type ResidualCapacityMap;
typedef boost::property_map<Graph, boost::edge_reverse_t>::type       ReverseEdgeMap;
typedef boost::graph_traits<Graph>::vertex_descriptor          Vertex;
typedef boost::graph_traits<Graph>::edge_descriptor            Edge;
typedef boost::graph_traits<Graph>::out_edge_iterator  OutEdgeIt; // Iterator

class EdgeAdder {
    Graph &G;
    EdgeCapacityMap &capacitymap;
    EdgeWeightMap &weightmap;
    ReverseEdgeMap  &revedgemap;

public:
    EdgeAdder(Graph &G, EdgeCapacityMap &capacitymap, EdgeWeightMap &weightmap, ReverseEdgeMap &revedgemap) 
        : G(G), capacitymap(capacitymap), weightmap(weightmap), revedgemap(revedgemap) {}

    void addEdge(int u, int v, long c, long w) {
        //std::cout << "Adding edge from " << u << " to " << v << " with cap " << c << " and cost " << w << std::endl;
        Edge e, rev_e;
        boost::tie(e, boost::tuples::ignore) = boost::add_edge(u, v, G);
        boost::tie(rev_e, boost::tuples::ignore) = boost::add_edge(v, u, G);
        capacitymap[e] = c;
        weightmap[e] = w; 
        capacitymap[rev_e] = 0;
        weightmap[rev_e] = -w; 
        revedgemap[e] = rev_e; 
        revedgemap[rev_e] = e; 
    }
};

void testcase() {
    int b, s, p;
    std::cin >> b >> s >> p;
    // Create Graph and Maps
    Graph G(b + s + 2);
    EdgeCapacityMap capacitymap = boost::get(boost::edge_capacity, G);
    EdgeWeightMap weightmap = boost::get(boost::edge_weight, G);
    ReverseEdgeMap revedgemap = boost::get(boost::edge_reverse, G);
    ResidualCapacityMap rescapacitymap = boost::get(boost::edge_residual_capacity, G);
    EdgeAdder eaG(G, capacitymap, weightmap, revedgemap);
    
    const int v_source = b + s;
    const int v_target = b + s + 1;
    std::vector<std::vector<int>> bipart(b, std::vector<int>(s, 0));
    for(int i = 0; i < b; i++){
        eaG.addEdge(v_source, i, 1, 0);
    }
    for(int i = 0; i < s; i++){
        eaG.addEdge(b+i, v_target, 1, 0);
    }
    for(int i = 0; i < p; i++){
        int from, to, coeff;
        std::cin >> from >> to >> coeff;
        bipart[from][to] = coeff;
    }
    for(int i = 0; i < b; i++){
        for(int j = 0; j < s; j++){
            eaG.addEdge(i, j+b, 1, -bipart[i][j]);
        }
    }

    int flow = boost::push_relabel_max_flow(G, v_source, v_target);
    boost::cycle_canceling(G);
    int cost = boost::find_flow_cost(G);
    std::cout << -cost << std::endl;
}

int main(){
    int t;
    std::cin >> t;
    while(t--){
        testcase();
    }
    return 0;
}