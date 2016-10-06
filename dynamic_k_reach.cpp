#include <fstream>
#include <algorithm>
#include <queue>
#include "dynamic_k_reach.h"

using namespace std;

void dynamic_k_reach::generate_cover() {
    /*for (const auto &u : out_neighbors){
        if (!cover.count(u.first)){
            for (const auto &v : u.second){
                if (!cover.count(v)){
                    cover.insert({u.first, v});
                    break;
                }
            }
        }
    }*/
    cover.insert({2, 4, 7, 9});
}

void dynamic_k_reach::construct_index(string filename,
                                      weight_t k) {
    this->k = k;
    ifstream fin(filename);

    vertex_t s, t;
    while (fin >> s >> t) {
        out_neighbors[s].insert(t);
        out_neighbors[t];
        in_neighbors[t].insert(s);
        in_neighbors[s];
    }

    fin.close();
    generate_cover();
    for (const auto &v : cover) {
        bfs_index(v);
    }
}

void dynamic_k_reach::insert_edge(vertex_t s, vertex_t t) {
    out_neighbors[s].insert(t);
    in_neighbors[t].insert(s);
    if (!cover.count(s) && !cover.count(t)) {
        if (!any_of(out_neighbors[s].begin(), out_neighbors[s].end(),
                    [this](const uint32_t v) { return cover.count(v); })) {
            // TODO replace with index-based bfs
            cover.insert(s);
            bfs_index(s);
        } else {
            cover.insert(t);
            bfs_index(t);
        }
    }

    if (cover.count(s) && cover.count(t)) {
        queue<vertex_t> frontier;
        set<vertex_t> visited;

        frontier.push(s);
        visited.insert(s);
        out_index[s][t] = 1;
        in_index[t][s] = 1;
        weight_t cur_level = 1, next_level = 0;
        for (weight_t level = 1; level < k && !frontier.empty();) {
            vertex_t u = frontier.front();
            frontier.pop();
            for (const auto &v : in_neighbors.at(u)) {
                if (!visited.count(v)) {
                    visited.insert(v);
                    frontier.push(v);
                    ++next_level;
                    if (cover.count(v)) {
                        out_index[v][t] = level + 1;
                        in_index[t][v] = level + 1;
                    }
                }
            }
            if (!--cur_level) {
                swap(cur_level, next_level);
                ++level;
            }
        }
    }
}

void dynamic_k_reach::remove_edge(vertex_t s, vertex_t t) {
    out_neighbors[s].erase(t);
    in_neighbors[t].erase(s);

    queue<vertex_t> frontier;
    cover_t visited;

    frontier.push(s);
    visited.insert(s);
    if (cover.count(s)) {
        out_index.erase(s);
        in_index.erase(s);
        bfs_index(s);
    }
    weight_t cur_level = 1, next_level = 0;
    for (weight_t level = 1; level < k && !frontier.empty();) {
        vertex_t u = frontier.front();
        frontier.pop();
        for (const auto &v : in_neighbors.at(u)) {
            if (!visited.count(v)) {
                visited.insert(v);
                frontier.push(v);
                ++next_level;
                if (cover.count(v)) {
                    out_index.erase(v);
                    in_index.erase(v);
                    bfs_index(v);
                }
            }
        }
        if (!--cur_level) {
            swap(cur_level, next_level);
            ++level;
        }
    }
}

void dynamic_k_reach::insert_vertex(vertex_t v) {
    in_neighbors[v];
    out_neighbors[v];
}

void dynamic_k_reach::remove_vertex(vertex_t v) {
    const auto in_nei = in_neighbors.at(v);
    in_neighbors.erase(v);
    out_neighbors.erase(v);
    if (cover.count(v)) {
        cover.erase(v);
        out_index.erase(v);
        in_index.erase(v);
    }
    for (const auto &i : in_nei) {
        remove_edge(i, v);
    }
}

bool dynamic_k_reach::intersect_adj(const graph_adj_t &graph_adj,
                                    const index_adj_t &index_adj,
                                    const weight_t weight) const {
    auto graph_it = graph_adj.begin();
    auto index_it = index_adj.begin();
    while (graph_it != graph_adj.end() && index_it != index_adj.end()) {
        if (*graph_it == index_it->first) {
            return index_it->second <= weight;
        } else if (*graph_it < index_it->first) {
            ++graph_it;
        } else {
            ++index_it;
        }
    }
    return false;
}

bool dynamic_k_reach::query_reachability(vertex_t s, vertex_t t) const {
    if (cover.count(s) && cover.count(t)) { // case 1
        const auto &index = out_index.at(s);
        return index.find(t) != index.end();
    } else if (cover.count(s) && !cover.count(t)) { // case 2
        return intersect_adj(in_neighbors.at(t), out_index.at(s), k - 1);
    } else if (!cover.count(s) && cover.count(t)) { // case 3
        return intersect_adj(out_neighbors.at(s), in_index.at(t), k - 1);
    } else { // case 4
        for (const auto &i : out_neighbors.at(s)) {
            if (intersect_adj(in_neighbors.at(t), out_index.at(i), k - 2)) {
                return true;
            }
        }
        return false;
    }
}

void dynamic_k_reach::bfs_index(vertex_t s) {
    queue<vertex_t> frontier;
    cover_t visited;
    out_index[s];
    in_index[s];
    frontier.push(s);
    visited.insert(s);
    weight_t cur_level = 1, next_level = 0;
    for (weight_t level = 0; level < k && !frontier.empty();) {
        vertex_t u = frontier.front();
        frontier.pop();
        for (const auto &v : out_neighbors.at(u)) {
            if (!visited.count(v)) {
                visited.insert(v);
                frontier.push(v);
                ++next_level;
                if (cover.count(v)) {
                    out_index[s][v] = level + 1;
                    in_index[v][s] = level + 1;
                }
            }
        }
        if (!--cur_level) {
            swap(cur_level, next_level);
            ++level;
        }
    }
}