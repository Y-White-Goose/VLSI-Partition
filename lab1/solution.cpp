#include "solution.h"
#include <algorithm>
#include <cmath>
#include <cstring>

void Solution::read_benchmark(Graph &graph, string benchmark_name) {
    ifstream file(benchmark_name);

    if(!file.is_open()) {
        cerr << "Failed to open the file!" << endl;
        exit(-1);
    }

    int edge_num, node_num;
    string line;
    getline(file >> ws, line);
    istringstream iss(line);
    iss >> edge_num;
    iss >> node_num;

    
    for(int i = 0; i < edge_num; i++) {
        getline(file, line);
        istringstream iss(line);
        int node_id;
        
        Net *net = graph.add_net(i);

        while(iss >> node_id) {
            Node *node = graph.get_or_create_node(node_id);
            node->add_net(net);
            net->add_node(node);
        }
        
    }

    file.close();
}

// ============================================================================
// 桶操作 (Bucket Array Operations)
// ============================================================================

void Solution::bucket_insert(int node_id, int side) {
    int idx = gain_[node_id] + gain_offset_;
    // 插入到链表头部
    bucket_prev_[node_id] = -1;
    bucket_next_[node_id] = bucket_heads_[side][idx];
    if (bucket_heads_[side][idx] != -1) {
        bucket_prev_[bucket_heads_[side][idx]] = node_id;
    }
    bucket_heads_[side][idx] = node_id;
    // 更新该侧最大增益
    if (gain_[node_id] > max_gain_[side]) {
        max_gain_[side] = gain_[node_id];
    }
}

void Solution::bucket_remove(int node_id, int side) {
    int idx = gain_[node_id] + gain_offset_;
    // 从双向链表中移除
    if (bucket_prev_[node_id] != -1) {
        bucket_next_[bucket_prev_[node_id]] = bucket_next_[node_id];
    } else {
        // 该节点是链表头
        bucket_heads_[side][idx] = bucket_next_[node_id];
        // 如果桶变空且是当前最大增益桶，向下扫描更新 max_gain_
        if (bucket_heads_[side][idx] == -1 && gain_[node_id] == max_gain_[side]) {
            while (max_gain_[side] >= -max_degree_ &&
                   bucket_heads_[side][max_gain_[side] + gain_offset_] == -1) {
                max_gain_[side]--;
            }
        }
    }
    if (bucket_next_[node_id] != -1) {
        bucket_prev_[bucket_next_[node_id]] = bucket_prev_[node_id];
    }
    bucket_next_[node_id] = -1;
    bucket_prev_[node_id] = -1;
}

int Solution::bucket_get_best_node(int side) {
    // 从最大增益开始向下扫描
    for (int g = max_gain_[side]; g >= -max_degree_; g--) {
        int idx = g + gain_offset_;
        int node_id = bucket_heads_[side][idx];
        // 遍历该增益桶的链表，找到第一个满足平衡约束的节点
        while (node_id != -1) {
            if (side == 0) {
                // 从 X 移动到 Y: X 大小减 1
                if (current_X_size_ > min_part_size_) {
                    return node_id;
                }
            } else {
                // 从 Y 移动到 X: X 大小加 1
                if (current_X_size_ < max_part_size_) {
                    return node_id;
                }
            }
            node_id = bucket_next_[node_id];
        }
    }
    return -1; // 没有满足平衡约束的节点
}

void Solution::update_node_gain(int node_id, int new_gain, int side) {
    int old_gain = gain_[node_id];
    if (old_gain == new_gain) return;
    bucket_remove(node_id, side);
    gain_[node_id] = new_gain;
    bucket_insert(node_id, side);
}

// ============================================================================
// 初始化与增益计算
// ============================================================================

void Solution::init_partition(Graph & /*graph*/) {
    // 将前一半节点放入 X (0), 后一半放入 Y (1)
    int half = total_nodes_ / 2;
    for (int i = 1; i <= total_nodes_; i++) {
        if (i <= half) {
            part_[i] = 0;
        } else {
            part_[i] = 1;
        }
    }
    current_X_size_ = half;
}

void Solution::compute_initial_gains(Graph &graph) {
    (void)graph; // 参数保留供将来使用

    // 重置超网分布计数
    fill(net_count_X_.begin(), net_count_X_.end(), 0);
    fill(net_count_Y_.begin(), net_count_Y_.end(), 0);

    // 统计每个超网在 X 和 Y 中的节点数
    for (Node *node : graph.get_nodes()) {
        int nid = node->get_index();
        for (Net *net : node->get_nets()) {
            int net_id = net->get_index();
            if (part_[nid] == 0)
                net_count_X_[net_id]++;
            else
                net_count_Y_[net_id]++;
        }
    }

    // 计算每个节点的初始增益
    for (Node *node : graph.get_nodes()) {
        int nid = node->get_index();
        int g = 0;
        for (Net *net : node->get_nets()) {
            int net_id = net->get_index();
            int F = net_count_X_[net_id]; // 该超网在 X 中的节点数
            int T = net_count_Y_[net_id]; // 该超网在 Y 中的节点数

            if (part_[nid] == 0) {
                // 节点在 X 中，计算移动到 Y 的增益
                // 若 T==0 且 F>1: 移动后超网被切割 → 增益 -1
                if (T == 0 && F > 1) g--;
                // 若 T>0 且 F==1: 移动后超网不再被切割 → 增益 +1
                if (T > 0 && F == 1) g++;
            } else {
                // 节点在 Y 中，计算移动到 X 的增益
                // 若 F==0 且 T>1: 移动后超网被切割 → 增益 -1
                if (F == 0 && T > 1) g--;
                // 若 F>0 且 T==1: 移动后超网不再被切割 → 增益 +1
                if (F > 0 && T == 1) g++;
            }
        }
        gain_[nid] = g;
    }
}

void Solution::build_buckets() {
    // 重置所有桶
    for (int s = 0; s < 2; s++) {
        fill(bucket_heads_[s].begin(), bucket_heads_[s].end(), -1);
        max_gain_[s] = -max_degree_ - 1;
    }
    fill(bucket_next_.begin(), bucket_next_.end(), -1);
    fill(bucket_prev_.begin(), bucket_prev_.end(), -1);

    // 将所有未锁定节点插入对应侧的桶
    for (int i = 1; i <= total_nodes_; i++) {
        if (!locked_[i]) {
            bucket_insert(i, part_[i]);
        }
    }
}

// ============================================================================
// 局部增益更新 (移动节点后更新受影响邻居的增益)
// ============================================================================

void Solution::update_gains_after_move(Graph &graph, int moved_node, int from_side) {
    Node *base = graph.get_node(moved_node);

    for (Net *net : base->get_nets()) {
        int net_id = net->get_index();
        int F = net_count_X_[net_id]; // 移动前的 X 侧计数
        int T = net_count_Y_[net_id]; // 移动前的 Y 侧计数

        // 更新超网分布计数
        if (from_side == 0) {
            net_count_X_[net_id]--;
            net_count_Y_[net_id]++;
        } else {
            net_count_X_[net_id]++;
            net_count_Y_[net_id]--;
        }

        // 遍历该超网中的所有节点，更新增益
        for (Node *cell : net->get_nodes()) {
            int cid = cell->get_index();
            if (locked_[cid]) continue; // 已锁定的节点不更新

            int delta = 0;

            if (from_side == 0) {
                // 基节点从 X 移动到 Y
                if (part_[cid] == 0) {
                    // 受影响节点在 X 中
                    if (F == 2) {
                        delta = (T == 0) ? 2 : 1;
                    } else if (F > 2 && T == 0) {
                        delta = 1;
                    }
                } else {
                    // 受影响节点在 Y 中
                    if (T == 1) {
                        delta = (F == 1) ? -2 : -1;
                    } else if (T > 1 && F == 1) {
                        delta = -1;
                    }
                }
            } else {
                // 基节点从 Y 移动到 X (对称情况)
                if (part_[cid] == 1) {
                    // 受影响节点在 Y 中
                    if (T == 2) {
                        delta = (F == 0) ? 2 : 1;
                    } else if (T > 2 && F == 0) {
                        delta = 1;
                    }
                } else {
                    // 受影响节点在 X 中
                    if (F == 1) {
                        delta = (T == 1) ? -2 : -1;
                    } else if (F > 1 && T == 1) {
                        delta = -1;
                    }
                }
            }

            if (delta != 0) {
                int new_gain = gain_[cid] + delta;
                update_node_gain(cid, new_gain, part_[cid]);
            }
        }
    }
}

// ============================================================================
// 单次 FM Pass
// ============================================================================

void Solution::fm_pass(Graph &graph) {
    // 解锁所有节点
    fill(locked_.begin(), locked_.end(), false);

    // 重新计算增益 (因为划分可能在上一次 pass 后发生了变化)
    compute_initial_gains(graph);

    // 构建桶结构
    build_buckets();

    // 记录移动序列，用于回溯
    vector<int> moves;           // 按顺序记录被移动的节点 id
    vector<int> gains_at_step;   // 每次移动后的累计增益
    int cumulative_gain = 0;
    int best_gain = 0;
    int best_step = 0;           // 达到最佳累计增益时的移动步数

    while (true) {
        // 分别从 X 侧和 Y 侧获取最佳候选节点
        int best_X = bucket_get_best_node(0);
        int best_Y = bucket_get_best_node(1);

        int chosen = -1;
        int from_side = -1;

        if (best_X != -1 && best_Y != -1) {
            // 两侧都有候选，选择增益更大的
            if (gain_[best_X] >= gain_[best_Y]) {
                chosen = best_X;
                from_side = 0;
            } else {
                chosen = best_Y;
                from_side = 1;
            }
        } else if (best_X != -1) {
            chosen = best_X;
            from_side = 0;
        } else if (best_Y != -1) {
            chosen = best_Y;
            from_side = 1;
        } else {
            break; // 没有满足平衡约束的可移动节点
        }

        // 从桶中移除该节点
        bucket_remove(chosen, from_side);

        // 执行移动
        part_[chosen] = 1 - from_side;
        locked_[chosen] = true;
        if (from_side == 0)
            current_X_size_--;
        else
            current_X_size_++;

        // 累计增益
        cumulative_gain += gain_[chosen];
        moves.push_back(chosen);
        gains_at_step.push_back(cumulative_gain);

        // 记录最佳累计增益位置
        if (cumulative_gain > best_gain) {
            best_gain = cumulative_gain;
            best_step = (int)moves.size();
        }

        // 局部更新受影响邻居的增益
        update_gains_after_move(graph, chosen, from_side);
    }

    // 回溯：撤销 best_step 之后的所有移动
    for (int i = (int)moves.size() - 1; i >= best_step; i--) {
        int node_id = moves[i];
        // 恢复原始划分
        if (part_[node_id] == 0) {
            part_[node_id] = 1;
            current_X_size_--;
        } else {
            part_[node_id] = 0;
            current_X_size_++;
        }
    }
}

// ============================================================================
// 主入口：多 Pass FM 算法
// ============================================================================

void Solution::my_partition_algorithm(Graph &graph, set<int> &X, set<int> &Y) {
    total_nodes_ = graph.get_node_num();

    // 计算最大度数 (用于确定桶数组大小)
    max_degree_ = 0;
    for (Node *node : graph.get_nodes()) {
        max_degree_ = max(max_degree_, (int)node->get_nets().size());
    }
    // 确保至少为 1，避免桶大小为 0
    if (max_degree_ < 1) max_degree_ = 1;
    gain_offset_ = max_degree_;

    // 分配数组 (1-indexed，所以大小为 N+1)
    part_.assign(total_nodes_ + 1, 0);
    gain_.assign(total_nodes_ + 1, 0);
    locked_.assign(total_nodes_ + 1, false);
    bucket_next_.assign(total_nodes_ + 1, -1);
    bucket_prev_.assign(total_nodes_ + 1, -1);

    int num_nets = graph.get_net_num();
    net_count_X_.assign(num_nets, 0);
    net_count_Y_.assign(num_nets, 0);

    // 初始化桶头数组: 2 侧 × (2*max_degree_ + 1) 个槽位
    int bucket_size = 2 * max_degree_ + 1;
    for (int s = 0; s < 2; s++) {
        bucket_heads_[s].assign(bucket_size, -1);
    }

    // 平衡约束
    min_part_size_ = (int)ceil(0.48 * total_nodes_);
    max_part_size_ = (int)floor(0.52 * total_nodes_);

    // 初始划分
    init_partition(graph);

    // 初始化超网分布计数
    fill(net_count_X_.begin(), net_count_X_.end(), 0);
    fill(net_count_Y_.begin(), net_count_Y_.end(), 0);
    for (Node *node : graph.get_nodes()) {
        int nid = node->get_index();
        for (Net *net : node->get_nets()) {
            int net_id = net->get_index();
            if (part_[nid] == 0)
                net_count_X_[net_id]++;
            else
                net_count_Y_[net_id]++;
        }
    }

    // 多 Pass 迭代
    int pass = 0;
    while (true) {
        // 计算当前 cut size
        int prev_cut = 0;
        for (int nid = 0; nid < num_nets; nid++) {
            if (net_count_X_[nid] > 0 && net_count_Y_[nid] > 0) prev_cut++;
        }

        fm_pass(graph);

        // fm_pass 内部的回溯会恢复节点划分但不会恢复超网计数，
        // 因此需要从当前划分重新计算超网分布
        fill(net_count_X_.begin(), net_count_X_.end(), 0);
        fill(net_count_Y_.begin(), net_count_Y_.end(), 0);
        for (Node *node : graph.get_nodes()) {
            int nid = node->get_index();
            for (Net *net : node->get_nets()) {
                int net_id = net->get_index();
                if (part_[nid] == 0)
                    net_count_X_[net_id]++;
                else
                    net_count_Y_[net_id]++;
            }
        }

        // 重新计算 cut size
        int curr_cut = 0;
        for (int nid = 0; nid < num_nets; nid++) {
            if (net_count_X_[nid] > 0 && net_count_Y_[nid] > 0) curr_cut++;
        }

        int improvement = prev_cut - curr_cut;
        cout << "Pass " << pass << ": cut = " << curr_cut
             << ", improvement = " << improvement << endl;

        if (improvement <= 0) break;
        pass++;
    }

    // 输出结果到 set
    X.clear();
    Y.clear();
    for (int i = 1; i <= total_nodes_; i++) {
        if (part_[i] == 0)
            X.insert(i);
        else
            Y.insert(i);
    }
}
