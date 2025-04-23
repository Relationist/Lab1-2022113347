#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <cctype>
#include <algorithm>
#include <cstdlib>
#include <queue>
#include <ctime>
using namespace std;
#define MODE 1
  
string cleanText(const string& text);
//修改1 
class Graph {
public:
    unordered_map<string, unordered_map<string, int>> adj;

    void addEdge(const string& from, const string& to) {
        adj[from][to]++;
        // 确保终点单词也出现在图中，即使它没有出边
        if (adj.find(to) == adj.end()) {
            adj[to] = unordered_map<string, int>();
        }
    }


    //void display() const {
    //    cout << "\n==== Graph ====\n";
    //    for (const auto& pair : adj) {
    //        const string& from = pair.first;
    //        for (const auto& to_pair : pair.second) {
    //            cout << from << " -> " << to_pair.first << " (" << to_pair.second << ")\n";
    //        }
    //    }
    //    cout << "================\n";
    //}

    void display() const {
        // 创建一个字符串流来构建 Graphviz DOT 格式
        stringstream ss;
        ss << "digraph G {\n";  // Graphviz 的开头

        // 遍历图的邻接表，生成节点和边的描述
        for (const auto& pair : adj) {
            const string& from = pair.first;
            for (const auto& to_pair : pair.second) {
                ss << "    \"" << from << "\" -> \"" << to_pair.first << "\" [label=\"" << to_pair.second << "\"];\n";
            }
        }

        ss << "}";  // Graphviz 的结尾

        // 将生成的 DOT 格式内容保存到一个 .dot 文件
        string dotContent = ss.str();
        ofstream outFile("graph.dot");
        outFile << dotContent;
        outFile.close();

        // 使用系统命令调用 Graphviz 工具生成 PNG 图像
        system("\"C:\\Program Files\\Graphviz\\bin\\dot.exe\" -Tpng graph.dot -o graph.png");
        system("start graph.png");


        cout << "Graph has been saved as 'graph.png'" << endl;
    }

    bool hasWord(const string& word) const {
        return adj.count(word) > 0;
    }

    vector<string> findBridgeWords(const string& word1, const string& word2) const {
        vector<string> bridges;
        if (!hasWord(word1) || !hasWord(word2)) {
            return bridges;
        }
        for (const auto& mid : adj.at(word1)) {
            const string& word3 = mid.first;
            if (adj.count(word3) && adj.at(word3).count(word2)) {
                bridges.push_back(word3);
            }
        }
        return bridges;
    }

    string generateNewText(const string& inputText) const {
        stringstream ss(cleanText(inputText));
        vector<string> words;
        string word;
        while (ss >> word) {
            words.push_back(word);
        }
        if (words.empty()) return "";

        string result = words[0];
        for (size_t i = 1; i < words.size(); ++i) {
            string w1 = words[i - 1], w2 = words[i];
            vector<string> bridges = findBridgeWords(w1, w2);
            if (!bridges.empty()) {
                string bridge = bridges[rand() % bridges.size()];
                result += " " + bridge;
            }
            result += " " + w2;
        }
        return result;
    }
    string generateGraphviz() const {
        stringstream ss;
        ss << "digraph G {\n"; // 开始定义有向图

        // 遍历图中的所有边
        for (const auto& pair : adj) {
            const string& from = pair.first;
            for (const auto& to_pair : pair.second) {
                const string& to = to_pair.first;
                int weight = to_pair.second;

                // 在Graphviz中每条边有一个权重
                ss << "    \"" << from << "\" -> \"" << to << "\" [label=\"" << weight << "\"];\n";
            }
        }

        ss << "}\n"; // 结束有向图的定义
        return ss.str();
    }

    void saveGraphAsImage(const string& filename) const {
        string graphviz = generateGraphviz(); // 获取Graphviz格式的字符串

        // 将Graphviz字符串写入临时文件
        string tempGraphFile = "temp.dot";
        ofstream tempFile(tempGraphFile);
        tempFile << graphviz;
        tempFile.close();

        // 使用 Graphviz 的 dot 工具生成 PNG 图像
        string command = "dot -Tpng " + tempGraphFile + " -o " + filename;
        system(command.c_str()); // 执行命令
    }

    pair<vector<string>, int> shortestPath(const string& start, const string& end) const {
        unordered_map<string, int> dist;
        unordered_map<string, string> prev;
        auto cmp = [&](const string& a, const string& b) { return dist[a] > dist[b]; };
        priority_queue<string, vector<string>, decltype(cmp)> pq(cmp);

        for (const auto& p : adj) {
            dist[p.first] = INT_MAX;
        }

        if (!hasWord(start) || !hasWord(end)) return { {}, -1 };

        dist[start] = 0;
        pq.push(start);

        while (!pq.empty()) {
            string u = pq.top(); pq.pop();
            for (const auto& neighbor : adj.at(u)) {
                string v = neighbor.first;
                int weight = neighbor.second;
                if (dist[u] + weight < dist[v]) {
                    dist[v] = dist[u] + weight;
                    prev[v] = u;
                    pq.push(v);
                }
            }
        }

        if (dist[end] == INT_MAX) return { {}, -1 };

        vector<string> path;
        for (string at = end; at != start; at = prev[at]) {
            path.push_back(at);
        }
        path.push_back(start);
        reverse(path.begin(), path.end());
        return { path, dist[end] };
    }

    unordered_map<string, double> calculatePageRank(double d = 0.85, int maxIter = 100) const {
        unordered_map<string, double> pr;
        double init = 1.0 / adj.size();
        for (const auto& p : adj) {
            pr[p.first] = init;
        }

        for (int iter = 0; iter < maxIter; ++iter) {
            unordered_map<string, double> newPr;
            for (const auto& p : adj) {
                newPr[p.first] = (1 - d) / adj.size();
            }
            for (const auto& p : adj) {
                double distribute = pr[p.first] / p.second.size();
                for (const auto& neighbor : p.second) {
                    newPr[neighbor.first] += d * distribute;
                }
            }
            pr = newPr;
        }

        return pr;
    }

    vector<string> randomWalk() const {
        srand(time(0));
        vector<string> walk;
        if (adj.empty()) return walk;  // 如果图为空，返回空路径

        // 随机选择一个起始节点
        auto it = adj.begin();
        advance(it, rand() % adj.size());  // 随机访问一个节点
        string current = it->first;
        unordered_set<string> visited_edges;  // 用于记录已访问的边

        walk.push_back(current);

        while (true) {
            // 使用 find 来避免 std::out_of_range 异常
            auto it = adj.find(current);
            if (it == adj.end() || it->second.empty()) {
                break;  // 如果当前节点没有邻居，结束随机游走
            }

            vector<string> neighbors;
            for (const auto& p : it->second) {
                neighbors.push_back(p.first);  // 获取当前节点的所有邻居
            }

            // 如果没有邻居，结束游走
            if (neighbors.empty()) {
                break;
            }

            string next = neighbors[rand() % neighbors.size()];  // 随机选择一个邻居
            string edge = current + "->" + next;

            // 如果已访问过这个边，结束游走
            if (visited_edges.count(edge)) {
                break;
            }

            visited_edges.insert(edge);  // 记录已访问的边
            walk.push_back(next);  // 继续随机游走
            current = next;  // 更新当前节点
        }

        return walk;  // 返回随机游走路径
    }


};

string cleanText(const string& text) {
    string result;
    for (char ch : text) {
        if (isalpha(ch)) {
            result += tolower(ch);
        }
        else {
            result += ' ';
        }
    }
    return result;
}

Graph buildGraphFromFile(const string& filename) {
    ifstream fin(filename);
    if (!fin.is_open()) {
        cerr << "无法打开文件：" << filename << endl;
        exit(1);
    }

    stringstream buffer;
    buffer << fin.rdbuf();
    string text = buffer.str();

    text = cleanText(text);

    stringstream ss(text);
    string word, prev_word;
    Graph g;

    while (ss >> word) {
        if (!prev_word.empty()) {
            g.addEdge(prev_word, word);
        }
        prev_word = word;
    }

    return g;
}

int main() {
    string filename;
    filename = "input.txt";
    cout << "文件名:input.txt）：";
    //cout << "请输入文本文件名（例如 input.txt）：";
    //getline(cin, filename);

    Graph graph = buildGraphFromFile(filename);

    while (true) {
        cout << "\n功能菜单:\n";
        cout << "1. 展示图\n2. 查询桥接词\n3. 生成新文本\n4. 查询最短路径\n5. 计算PageRank\n6. 随机游走\n0. 退出\n";
        cout << "请输入功能编号：";
        int choice;
        cin >> choice;
        cin.ignore();

        if (choice == 0) break;

        if (choice == 1) {
            graph.display();
        }
        else if (choice == 2) {
            string word1, word2;
            cout << "输入两个单词：";
            cin >> word1 >> word2;
            transform(word1.begin(), word1.end(), word1.begin(), ::tolower);
            transform(word2.begin(), word2.end(), word2.begin(), ::tolower);
            vector<string> bridges = graph.findBridgeWords(word1, word2);
            if (!graph.hasWord(word1) || !graph.hasWord(word2)) {
                cout << "No " << (graph.hasWord(word1) ? word2 : word1) << " in the graph!\n";
            }
            else if (bridges.empty()) {
                cout << "No bridge words from " << word1 << " to " << word2 << "!\n";
            }
            else {
                cout << "The bridge words from " << word1 << " to " << word2 << " are: ";
                for (size_t i = 0; i < bridges.size(); ++i) {
                    cout << bridges[i];
                    if (i != bridges.size() - 1) cout << ", ";
                }
                cout << ".\n";
            }
        }
        else if (choice == 3) {
            string inputText;
            cout << "输入一句文本：";
            getline(cin, inputText);
            cout << "生成的新文本：" << graph.generateNewText(inputText) << "\n";
        }
        else if (choice == 4) {
            string word1, word2;
            cout << "输入起点和终点单词：";
            cin >> word1 >> word2;
            transform(word1.begin(), word1.end(), word1.begin(), ::tolower);
            transform(word2.begin(), word2.end(), word2.begin(), ::tolower);
            auto [path, dist] = graph.shortestPath(word1, word2);
            if (path.empty()) {
                cout << "不可达！\n";
            }
            else {
                cout << "最短路径：";
                for (size_t i = 0; i < path.size(); ++i) {
                    cout << path[i];
                    if (i != path.size() - 1) cout << " -> ";
                }
                cout << "，长度为：" << dist << "\n";
            }
        }
        else if (choice == 5) {
            auto pr = graph.calculatePageRank();
            cout << "PageRank结果：\n";
            for (const auto& p : pr) {
                cout << p.first << ": " << p.second << "\n";
            }
        }
        else if (choice == 6) {
            auto walk = graph.randomWalk();
            cout << "随机游走路径：";
            for (const auto& word : walk) {
                cout << word << " ";
            }
            cout << "\n";
        }
        else {
            cout << "无效选择，请重新输入！\n";
        }
    }

    return 0;
}
