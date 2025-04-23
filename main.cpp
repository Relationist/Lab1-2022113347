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
//�޸�1 
class Graph {
public:
    unordered_map<string, unordered_map<string, int>> adj;

    void addEdge(const string& from, const string& to) {
        adj[from][to]++;
        // ȷ���յ㵥��Ҳ������ͼ�У���ʹ��û�г���
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
        // ����һ���ַ����������� Graphviz DOT ��ʽ
        stringstream ss;
        ss << "digraph G {\n";  // Graphviz �Ŀ�ͷ

        // ����ͼ���ڽӱ����ɽڵ�ͱߵ�����
        for (const auto& pair : adj) {
            const string& from = pair.first;
            for (const auto& to_pair : pair.second) {
                ss << "    \"" << from << "\" -> \"" << to_pair.first << "\" [label=\"" << to_pair.second << "\"];\n";
            }
        }

        ss << "}";  // Graphviz �Ľ�β

        // �����ɵ� DOT ��ʽ���ݱ��浽һ�� .dot �ļ�
        string dotContent = ss.str();
        ofstream outFile("graph.dot");
        outFile << dotContent;
        outFile.close();

        // ʹ��ϵͳ������� Graphviz �������� PNG ͼ��
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
        ss << "digraph G {\n"; // ��ʼ��������ͼ

        // ����ͼ�е����б�
        for (const auto& pair : adj) {
            const string& from = pair.first;
            for (const auto& to_pair : pair.second) {
                const string& to = to_pair.first;
                int weight = to_pair.second;

                // ��Graphviz��ÿ������һ��Ȩ��
                ss << "    \"" << from << "\" -> \"" << to << "\" [label=\"" << weight << "\"];\n";
            }
        }

        ss << "}\n"; // ��������ͼ�Ķ���
        return ss.str();
    }

    void saveGraphAsImage(const string& filename) const {
        string graphviz = generateGraphviz(); // ��ȡGraphviz��ʽ���ַ���

        // ��Graphviz�ַ���д����ʱ�ļ�
        string tempGraphFile = "temp.dot";
        ofstream tempFile(tempGraphFile);
        tempFile << graphviz;
        tempFile.close();

        // ʹ�� Graphviz �� dot �������� PNG ͼ��
        string command = "dot -Tpng " + tempGraphFile + " -o " + filename;
        system(command.c_str()); // ִ������
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
        if (adj.empty()) return walk;  // ���ͼΪ�գ����ؿ�·��

        // ���ѡ��һ����ʼ�ڵ�
        auto it = adj.begin();
        advance(it, rand() % adj.size());  // �������һ���ڵ�
        string current = it->first;
        unordered_set<string> visited_edges;  // ���ڼ�¼�ѷ��ʵı�

        walk.push_back(current);

        while (true) {
            // ʹ�� find ������ std::out_of_range �쳣
            auto it = adj.find(current);
            if (it == adj.end() || it->second.empty()) {
                break;  // �����ǰ�ڵ�û���ھӣ������������
            }

            vector<string> neighbors;
            for (const auto& p : it->second) {
                neighbors.push_back(p.first);  // ��ȡ��ǰ�ڵ�������ھ�
            }

            // ���û���ھӣ���������
            if (neighbors.empty()) {
                break;
            }

            string next = neighbors[rand() % neighbors.size()];  // ���ѡ��һ���ھ�
            string edge = current + "->" + next;

            // ����ѷ��ʹ�����ߣ���������
            if (visited_edges.count(edge)) {
                break;
            }

            visited_edges.insert(edge);  // ��¼�ѷ��ʵı�
            walk.push_back(next);  // �����������
            current = next;  // ���µ�ǰ�ڵ�
        }

        return walk;  // �����������·��
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
        cerr << "�޷����ļ���" << filename << endl;
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
    cout << "�ļ���:input.txt����";
    //cout << "�������ı��ļ��������� input.txt����";
    //getline(cin, filename);

    Graph graph = buildGraphFromFile(filename);

    while (true) {
        cout << "\n���ܲ˵�:\n";
        cout << "1. չʾͼ\n2. ��ѯ�ŽӴ�\n3. �������ı�\n4. ��ѯ���·��\n5. ����PageRank\n6. �������\n0. �˳�\n";
        cout << "�����빦�ܱ�ţ�";
        int choice;
        cin >> choice;
        cin.ignore();

        if (choice == 0) break;

        if (choice == 1) {
            graph.display();
        }
        else if (choice == 2) {
            string word1, word2;
            cout << "�����������ʣ�";
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
            cout << "����һ���ı���";
            getline(cin, inputText);
            cout << "���ɵ����ı���" << graph.generateNewText(inputText) << "\n";
        }
        else if (choice == 4) {
            string word1, word2;
            cout << "���������յ㵥�ʣ�";
            cin >> word1 >> word2;
            transform(word1.begin(), word1.end(), word1.begin(), ::tolower);
            transform(word2.begin(), word2.end(), word2.begin(), ::tolower);
            auto [path, dist] = graph.shortestPath(word1, word2);
            if (path.empty()) {
                cout << "���ɴ\n";
            }
            else {
                cout << "���·����";
                for (size_t i = 0; i < path.size(); ++i) {
                    cout << path[i];
                    if (i != path.size() - 1) cout << " -> ";
                }
                cout << "������Ϊ��" << dist << "\n";
            }
        }
        else if (choice == 5) {
            auto pr = graph.calculatePageRank();
            cout << "PageRank�����\n";
            for (const auto& p : pr) {
                cout << p.first << ": " << p.second << "\n";
            }
        }
        else if (choice == 6) {
            auto walk = graph.randomWalk();
            cout << "�������·����";
            for (const auto& word : walk) {
                cout << word << " ";
            }
            cout << "\n";
        }
        else {
            cout << "��Чѡ�����������룡\n";
        }
    }

    return 0;
}
